void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {

  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    _swap_int16_t(x0, y0);
    _swap_int16_t(x1, y1);
  }

  if (x0 > x1) {
    _swap_int16_t(x0, x1);
    _swap_int16_t(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0 <= x1; x0++) {
    if (steep) {
      display.drawPixel(y0, x0, color);
    } else {
      display.drawPixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}


void drawGraph2(int series[], int seriesDepth, int sx, int sy, int sh, int sw, int s_min, int s_max, int s_thresh) {
  if (VERBOSE) {
    Serial.println("in drawGraph2");
  }
  if (DEBUG) {
    Serial.print("seriesDepth ");
    Serial.print(seriesDepth);
    Serial.print(" sx ");
    Serial.print(sx);
    Serial.print(" sy ");
    Serial.print(sy);
    Serial.print(" sh ");
    Serial.print(sh);
    Serial.print(" sw ");
    Serial.print(sw);
    Serial.print(" s_min ");
    Serial.print(s_min);
    Serial.print(" s_max ");
    Serial.println(s_max);
  }
  int scaled_r = 0;
  for (int i = 0; i < seriesDepth; i++) {
    if (s_max - s_min < s_thresh) {
      s_max = (s_min + s_thresh);
    }
    int scaled_r = round(constrain(map(series[i], s_min, s_max, 0, sh), 0, sh));
    //    Serial.print(" scaled_r ");
    //    Serial.println(scaled_r);
    int lt = (sy + sh - scaled_r);
    //    Serial.print(" lt ");
    //    Serial.println(lt);
    drawLine(sx + i, (sy + sh), sx + i, lt, (i % TICK_INTERVAL ? GxEPD_BLACK : GxEPD_WHITE)); // add +1 o lineterm always to make a white border btwn graphs
  }

  display.setFont(&Org_01);
  display.setCursor(0, sy + 5);
  display.print(s_max);
  //  display.print("max");
  display.setCursor(0, sy + sh - 5);
  display.print(s_min);
  //  display.print("min");
}

void drawGraph() {
  if (VERBOSE) {
    Serial.println("in drawGraph");
  }

  int scaled_reading1 = 0;
  int scaled_reading2 = 0;

  for (int i = 0; i < SAMPLE_DEPTH; i++) {
    uint16_t tc = GxEPD_BLACK;
    if (i % TICK_INTERVAL) {
      tc = GxEPD_BLACK;
    } else {
      tc = GxEPD_WHITE;
    }
    if (series1max - series1min < SERIES1_SIGNIFICANCE_THRESHOLD) {
      scaled_reading1 = round(constrain(map(series1[i], series1min, (series1min + SERIES1_SIGNIFICANCE_THRESHOLD), 0, GRAPH_H), 0, GRAPH_H));
    } else {
      scaled_reading1 = round(constrain(map(series1[i], series1min, series1max, 0, GRAPH_H), 0, GRAPH_H));
    }

    if (series2max - series2min < SERIES2_SIGNIFICANCE_THRESHOLD) {
      scaled_reading2 = round(constrain(map(series2[i], series2min, (series2min + SERIES2_SIGNIFICANCE_THRESHOLD), 0, GRAPH_H), 0, GRAPH_H));
    } else {
      scaled_reading2 = round(constrain(map(series2[i], series2min, series2max, 0, GRAPH_H), 0, GRAPH_H));
    }
    // instead of display_height - scaled_reading1, we want to show decrease in reading as increase in graph height so:
    // check again that this is calculating the right height, seems like this is for 2/3 screen height not 1/3
    int lineterm1 = (GRAPH_Y * 3 - (GRAPH_H - scaled_reading1));
    int lineterm2 = (GRAPH_Y * 2 - (GRAPH_H - scaled_reading2));
    // height graph is along the bottom
    drawLine(GRAPH_X + i, GRAPH_Y * 3, GRAPH_X + i, lineterm1 + 1, tc); // add +1 o lineterm always to make a white border btwn graphs
    // gas graph is in the middle
    drawLine(GRAPH_X + i, GRAPH_Y * 2, GRAPH_X + i, lineterm2, tc);
  }

  display.setFont(&Org_01);
  display.setCursor(0, GRAPH_Y);
  if (series2max - series2min < SERIES2_SIGNIFICANCE_THRESHOLD) {
    display.print(series2min + SERIES2_SIGNIFICANCE_THRESHOLD);
  } else {
    display.print(series2max);
  }
  display.setCursor(0, GRAPH_Y * 2 - 5);
  display.print(series2min);
  display.setCursor(0, GRAPH_Y * 2 + 5);
  if (series1max - series1min < SERIES1_SIGNIFICANCE_THRESHOLD) {
    display.print(series1min + SERIES1_SIGNIFICANCE_THRESHOLD);
  } else {
    display.print(series1max);
  }
  display.setCursor(0, display.height() - 1);
  display.print(series1min);
}

void drawMessage() {
  if (VERBOSE) {
    Serial.println("in drawMessage");
  }
}

void drawName() {
  if (VERBOSE) {
    Serial.println("in drawName");
  }
  display.setCursor(NAME_X, NAME_Y);
  display.setFont(&FreeSansBoldOblique9pt7b);
  display.println(starterName);
}

void drawTrend() {
  // draw a weathervane-like line to show slope of latest readings
  if (VERBOSE) {
    Serial.println("in drawTrend");
  }
  // look at slope and show trend, up down or unch
  float theTrend = summarize(series2, TICK_INTERVAL * 2); // using TICK_INTERVAL as modulus here because it'll make natural sense re the graph
  Serial.print("the trend is ");
  Serial.println(theTrend);
  drawLine(TREND_X, TREND_Y / 2, TREND_X + TREND_W, constrain(((TICK_INTERVAL * 2) + (TICK_INTERVAL * -2) * theTrend), 0, TREND_Y / 2), GxEPD_BLACK);
}

void DrawUpdateStatus() {

}
