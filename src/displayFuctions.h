void showSatellites() {
  if ( gps.satellites.isValid() ) {

    int satCount = gps.satellites.value();
    int xpos;
    int ypos = 30;
    if ( oldSatCount != satCount) {
      uint16_t fg_color = TFT_BLACK;
      uint16_t bg_color = TFT_BLACK; 
      if ( satCount >= 4) {
        fg_color = TFT_GREEN;
      }
      else if ( satCount <2) {
        fg_color = TFT_RED;
      }
      else {
        fg_color = TFT_YELLOW;
      }

      
      uint16_t x = tft.width() / 2;  // Position of centre of arc
      uint16_t y = tft.height() / 2;

      uint8_t radius       = tft.width()/2; // Outer arc radius
      uint8_t thickness    = 10;     // Thickness
      uint8_t inner_radius = radius - thickness;        // Calculate inner radius (can be 0 for circle segment)

      // 0 degrees is at 6 o'clock position
      // Arcs are drawn clockwise from start_angle to end_angle
      uint16_t start_angle = 60; // Start angle must be in range 0 to 360
      uint16_t end_angle   = map(satCount, 0, 12, 61, 300); // End angle must be in range 0 to 360
      
      tft.setTextColor(TFT_BLACK, TFT_BLACK);
      tft.loadFont(AA_FONT_SMALL);
      xpos = (tft.width()/2) - (tft.textWidth((String)"Satellites: " + oldSatCount) / 2);
      // tft.setTextSize(4);
      tft.setCursor(xpos, ypos);
      tft.print("Satellites: ");
      tft.print(oldSatCount);

      tft.setTextColor(TFT_CYAN, TFT_BLACK);
      xpos = (tft.width()/2) - (tft.textWidth((String)"Satellites: " + satCount) / 2);
      // tft.setTextSize(4);
      tft.setCursor(xpos, ypos);
      tft.print("Satellites: ");
      tft.print(satCount);

      bool arc_end = true;           // true = round ends, false = square ends (arc_end parameter can be omitted, ends will then be square)

      tft.drawSmoothArc(x, y, radius, inner_radius, 0, 360, TFT_BLACK, TFT_BLACK, arc_end);
      tft.drawSmoothArc(x, y, radius, inner_radius, start_angle, end_angle, fg_color, bg_color, arc_end);
      oldSatCount = satCount;
    }
  }
}