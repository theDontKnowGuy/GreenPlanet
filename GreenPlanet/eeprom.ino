void writeString(char add, String data)
{

  int _size = data.length();
  logThis(3, "storing " + data + " \n of size: " + _size);

  int i;
  for (i = 0; i < _size; i++)
  {
    EEPROM.write(add + i, data[i]);
  }
  EEPROM.write(add + _size, '\0'); //Add termination null character for String Data
  EEPROM.commit();

  logThis(5, "validation: " + readEEPROM(add), 2);
}

String readEEPROM(char add)
{
  int i;
  char data[maxEEPROMMessageLength];
  int len = 0;
  unsigned char k;
  k = EEPROM.read(add);
  while (k != '\0' && len < maxEEPROMMessageLength) //Read until null character
  {
    k = EEPROM.read(add + len);
    data[len] = k;
    len++;
  }
  data[len] = '\0';

  return String(data);
}
