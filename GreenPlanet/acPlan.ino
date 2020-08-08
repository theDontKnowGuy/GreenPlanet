int execProtocolPlan(int power ,  int targetTemp, int ACProtocol) {

  logThis(1, "Running plan with power = " + String(power) + " and temp = " + String(targetTemp), 2);
  
  ac.next.protocol = decode_type_t::ELECTRA_AC;  // Set a protocol to use.
  ac.next.model = 1;  // Some A/Cs have different models. Try just the first.
  ac.next.mode = stdAc::opmode_t::kCool;  // Run in cool mode initially.
  ac.next.celsius = true;  // Use Celsius for temp units. False = Fahrenheit
  ac.next.degrees = targetTemp;
  ac.next.fanspeed = stdAc::fanspeed_t::kAuto;  // Start the fan at medium.
  ac.next.swingv = stdAc::swingv_t::kOff;  // Don't swing the fan up or down.
  ac.next.swingh = stdAc::swingh_t::kOff;  // Don't swing the fan left or right.
  ac.next.light = false;    // Turn off any LED/Lights/Display that we can.
  ac.next.beep = true;      // Turn off any beep from the A/C if we can.
  ac.next.econo = true;     // Turn off any economy modes if we can.
  ac.next.filter = false;   // Turn off any Ion/Mold/Health filters if we can.
  ac.next.turbo = false;    // Don't use any turbo/powerful/etc modes.
  ac.next.quiet = false;    // Don't use any quiet/silent/etc modes.
  ac.next.sleep = -1;       // Don't set any sleep time or modes.
  ac.next.clean = false;    // Turn off any Cleaning options if we can.
  ac.next.clock = -1;       // Don't set any current time if we can avoid it.
  //ac.next.power = false;  // Initially start with the unit off.

  ac.next.power = power;     // We want to turn on the A/C unit.

  logThis(2, "Now executing acPlan");

  for (int i = 0; i < 2; i++)
  {

    logThis(3, "Calling IR sequance", 1);
    ac.sendAc();
    digitalWrite(green, HIGH);
    vTaskDelay(50);
    digitalWrite(green, LOW);
    vTaskDelay(delayBetweenExecs * 3);
  }

  return 0;
}
