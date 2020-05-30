#if (defined(SERVER))

String buildRawCode(const decode_results *const results)
{
  String output = "";
  // Reserve some space for the string to reduce heap fragmentation.
  output.reserve(1536); // 1.5KB should cover most cases.
  // Start declaration
  output += F("{"); // Start declaration

  // Dump data
  for (uint16_t i = 1; i < results->rawlen; i++)
  {
    uint32_t usecs;
    for (usecs = results->rawbuf[i] * kRawTick; usecs > UINT16_MAX;
         usecs -= UINT16_MAX)
    {
      output += uint64ToString(UINT16_MAX);
      if (i % 2)
        output += F(", 0,  ");
      else
        output += F(",  0, ");
    }
    output += uint64ToString(usecs, 10);

    ACcode[i - 1] = usecs;

    if (i < results->rawlen - 1)
      output += kCommaSpaceStr; // ',' not needed on the last one
    if (i % 2 == 0)
      output += ' '; // Extra if it was even.
  }

  // End declaration
  output += F("};");

  writeString(10, output);

  return output;
}

#endif
