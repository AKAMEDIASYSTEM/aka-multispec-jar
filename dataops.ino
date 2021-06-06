float summarize(int farback) {
  float slope = ((series1[0] - series1[farback - 1]) / farback);
  return slope;
}

void initSensors() {
  if (! vl.begin()) {
    Serial.println("Sensor VL6180 not found :(");
    delay(1000);
  } else {
    Serial.println("Sensor VL6180 online");
  }

  if (!scd.begin()) {
    Serial.println("Sensor SCD30 not found :(");
    delay(1000);
  } else {
    Serial.println("Sensor SCD30 online");
  }

}

int maxOf(int series[], int seriesDepth) {
  int theMax = -200000;
  for (int i = 0; i < seriesDepth; i++) {
    if (series[i] > theMax) {
      theMax = series[i];
    }
  }
  return theMax;
}

int minOf(int series[], int seriesDepth) {
  int theMin = 200000;
  for (int i = 0; i < seriesDepth; i++) {
    if (series[i] < theMin) {
      theMin = series[i];
    }
  }
  return theMin;
}

void clobber() {
  // fill timeseries with dummy init data for debugging purposes
  for (int i = 0; i < SAMPLE_DEPTH; i++) {
    series1[i] = random(0, 300);
    series2[i] = random(400, 800);
  }
  Serial.println("Clobbered all data and filled with dummy data.");
  if (VERBOSE) {
    Serial.println("Clobbered all data and filled with dummy data.");
  }
}


void startFan() {
  if (mySwitch.isConnected()) {
    mySwitch.powerOn();
  } else {
    Serial.println("START FAN FAILED");
  }
  if (VERBOSE) {
    Serial.println("FAN ON");
  }
}

void stopFan() {
  if (mySwitch.isConnected()) {
    mySwitch.powerOff();
    mySwitch.powerOff();
    mySwitch.powerOff();
    mySwitch.powerOff();
    mySwitch.powerOff();
    mySwitch.powerOff();
    mySwitch.powerOff();
  } else {
    Serial.println("STOP FAN FAILED");
  }
  if (VERBOSE) {
    Serial.println("FAN OFF");
  }
}

void sampler() {
  int distance = 0;
  int gas = 0;
  int rh = 0;
  int tmp = 0;

  startFan();
  delay(FAN_DURATION_MS);

  if (VERBOSE) {
    Serial.print("inside sampler, series1[0] is ");
    Serial.println(series1[0]);
  }
  uint8_t range = vl.readRange();
  uint8_t status = vl.readRangeStatus();

  if (status == VL6180X_ERROR_NONE) {
    distance = range;
  }
  if (VERBOSE) {
    Serial.print("VL6180 distance is: ");
    Serial.println(distance);
  }
  if ((status >= VL6180X_ERROR_SYSERR_1) && (status <= VL6180X_ERROR_SYSERR_5)) {
    Serial.println("VL6180X_ERROR error");
  }
  else if (status == VL6180X_ERROR_ECEFAIL) {
    Serial.println("VL6180X_ERROR ECE failure");
  }
  else if (status == VL6180X_ERROR_NOCONVERGE) {
    Serial.println("VL6180X_ERROR No convergence");
  }
  else if (status == VL6180X_ERROR_RANGEIGNORE) {
    Serial.println("VL6180X_ERROR Ignoring range");
  }
  else if (status == VL6180X_ERROR_SNR) {
    Serial.println("VL6180X_ERROR Signal/Noise error");
  }
  else if (status == VL6180X_ERROR_RAWUFLOW) {
    Serial.println("Raw reading underflow");
  }
  else if (status == VL6180X_ERROR_RAWOFLOW) {
    Serial.println("VL6180X_ERROR Raw reading overflow");
  }
  else if (status == VL6180X_ERROR_RANGEUFLOW) {
    Serial.println("VL6180X_ERROR Range reading underflow");
  }
  else if (status == VL6180X_ERROR_RANGEOFLOW) {
    Serial.println("VL6180X_ERROR Range reading overflow");
  }
  if (distance > SENSOR_MAX) {
    distance = SENSOR_MAX;
  }
  for (int i = SAMPLE_DEPTH - 2; i >= 0; i--) {
    series1[i + 1] = series1[i];
  }
  series1[0] = distance;

  series1max = maxOf(series1, SAMPLE_DEPTH);
  series1min = minOf(series1, SAMPLE_DEPTH);

  if (distance > series1max_h) {
    if (VERBOSE) {
      Serial.print("new series1 historical maximum: ");
      Serial.println(distance);
    }
    series1max_h = distance;
  }
  if (distance < series1min_h) {
    if (VERBOSE) {
      Serial.print("new series1 historical minimum: ");
      Serial.println(distance);
    }
    series1min_h = distance;
  }

  //  if (VERBOSE) {
  //    for (int i = 0; i < SAMPLE_DEPTH; i++) {
  //      Serial.print(series1[i]);
  //      Serial.print(" ");
  //    }
  //    Serial.println();
  //  }

  // SERIES 2 OPERATIONS

  if (scd.dataReady()) {
    if (VERBOSE) {
      Serial.println("Data available!");
    }

    if (!scd.read()) {
      Serial.println("Error reading sensor data");
      return;
    }
    tmp = scd.temperature;
    rh = scd.relative_humidity;
    gas = scd.CO2;
  } else {
    Serial.println("error with scd30, dataReady was false");
  }
  for (int i = SAMPLE_DEPTH - 2; i >= 0; i--) {
    series2[i + 1] = series2[i];
    series3[i + 1] = series3[i];
    series4[i + 1] = series4[i];
  }
  series2[0] = gas;
  series3[0] = tmp;
  series4[0] = rh;

  series2max = maxOf(series2, SAMPLE_DEPTH);
  series2min = minOf(series2, SAMPLE_DEPTH);
  series3max = maxOf(series3, SAMPLE_DEPTH);
  series3min = minOf(series3, SAMPLE_DEPTH);
  series4max = maxOf(series4, SAMPLE_DEPTH);
  series4min = minOf(series4, SAMPLE_DEPTH);

  if (gas > series2max_h) {
    if (VERBOSE) {
      Serial.print("new s2 historical maximum: ");
      Serial.println(gas);
    }
    series2max_h = gas;
  }

  if (gas < series2min_h) {
    if (VERBOSE) {
      Serial.print("new s2 historical minimum: ");
      Serial.println(gas);
    }
    series2min_h = gas;
  }

  if (tmp > series3max_h) {
    if (VERBOSE) {
      Serial.print("new s3 historical maximum: ");
      Serial.println(tmp);
    }
    series3max_h = tmp;
  }

  if (tmp < series3min_h) {
    if (VERBOSE) {
      Serial.print("new s3 historical minimum: ");
      Serial.println(tmp);
    }
    series3min_h = tmp;
  }

  if (rh > series4max_h) {
    if (VERBOSE) {
      Serial.print("new s4 historical maximum: ");
      Serial.println(rh);
    }
    series4max_h = rh;
  }

  if (rh < series4min_h) {
    if (VERBOSE) {
      Serial.print("new s4 historical minimum: ");
      Serial.println(rh);
    }
    series4min_h = rh;
  }

  //  if (VERBOSE) {
  //    for (int i = 0; i < SAMPLE_DEPTH; i++) {
  //      Serial.print(series2[i]);
  //      Serial.print(" ");
  //    }
  //    Serial.println();
  //  }

  if (DEBUG) {

    Serial.print("distance is ");
    Serial.print(distance);
    Serial.print(" local min: ");
    Serial.print(series1min);
    Serial.print(" local max: ");
    Serial.print(series1max);
    Serial.print(" historical min: ");
    Serial.print(series1min_h);
    Serial.print(" historical max: ");
    Serial.println(series1max_h);
    
    Serial.print("gas is ");
    Serial.print(gas);
    Serial.print(" local min: ");
    Serial.print(series2min);
    Serial.print(" local max: ");
    Serial.print(series2max);
    Serial.print(" historical min: ");
    Serial.print(series2min_h);
    Serial.print(" historical max: ");
    Serial.println(series2max_h);

    Serial.print("tmp is ");
    Serial.print(tmp);
    Serial.print(" local min: ");
    Serial.print(series3min);
    Serial.print(" local max: ");
    Serial.print(series3max);
    Serial.print(" historical min: ");
    Serial.print(series3min_h);
    Serial.print(" historical max: ");
    Serial.println(series3max_h);

    Serial.print("rh is ");
    Serial.print(rh);
    Serial.print(" local min: ");
    Serial.print(series4min);
    Serial.print(" local max: ");
    Serial.print(series4max);
    Serial.print(" historical min: ");
    Serial.print(series4min_h);
    Serial.print(" historical max: ");
    Serial.println(series4max_h);
    Serial.println();
  }




  stopFan();
}
