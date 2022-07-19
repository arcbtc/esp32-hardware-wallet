//========================================================================//
//================================COMMANDS================================//
//========================================================================//

String encrytptedSeed = "";
String passwordHash = "";

String keyHash = ""; // todo: revisit

bool authenticated = false;
String command = "";
String commandData = "";

String message = "Welcome";
String subMessage = "Enter password";

String serialData = "";

void run() {
  if (loadFiles() == false) {
    message = "Failed opening files";
    subMessage = "Reset or 'help'";
  }

  if (message != "" || subMessage != "")
    showMessage(message, subMessage);

  serialData = awaitSerialData();
  int spacePos = serialData.indexOf(" ");
  command = serialData.substring(0, spacePos);
  if (spacePos == -1) {
    commandData = "";
  } else {
    commandData = serialData.substring(spacePos + 1, serialData.length());
  }

  executeCommand(command, commandData);

  delay(DELAY_MS);
}


void executeCommand(String command, String commandData) {
  if (command == COMMAND_HELP) {
    executeHelp(commandData);
  } else if (command == COMMAND_WIPE) {
    executeWipeHww(commandData);
  } else if (command == COMMAND_PASSWORD) {
    executePasswordCheck(commandData);
  } else if (command == COMMAND_SEED) {
    executeShowSeed(commandData);
  } else if (command == COMMAND_SIGN_PSBT) {
    executeSignPsbt(commandData);
  } else if (command == COMMAND_RESTORE) {
    executeRestore(commandData);
  } else {
    executeUnknown(commandData);
  }
}

void executeHelp(String commandData) {
  help();
}

void executePasswordCheck(String commandData) {
  if (commandData == "") {
    showMessage("Enter password", "8 numbers/letters");
    commandData = awaitSerialData();
  }
  String passwordHash = hashPassword(commandData);
  if (passwordHash == keyHash) {
    authenticated = true;
    message = "Password correct!";
    subMessage = "Ready to sign sir!";
  } else {
    authenticated = false;
    message = "Wrong password, try again";
    subMessage = "8 numbers/letters";
  }
}

void executeWipeHww(String commandData) {
  showMessage("Resetting...", "");
  delay(2000);

  if (commandData == "") {
    showMessage("Enter new password", "8 numbers/letters");
    commandData = awaitSerialData();
  }

  authenticated = wipeHww(commandData);
  if (authenticated == true) {
    message = "Successfully wiped!";
    subMessage = "Every new beginning comes from some other beginning's end.";
  } else {
    message = "Error, try again";
    subMessage = "8 numbers/letters";
  }
}

void executeShowSeed(String commandData) {
  if (authenticated == false) {
    message = "Enter password!";
    subMessage = "8 numbers/letters";
    return;
  }
  message = "";
  subMessage = "";
  printMnemonic(encrytptedSeed);

}

void executeRestore(String commandData) {
  if (commandData == "") {
    showMessage("Enter seed words", "Separated by spaces");
    commandData = awaitSerialData();
  }
  int size = getMnemonicBytes(commandData);
  if (size == 0) {
    message = "Wrong word count!";
    subMessage = "Must be 12, 15, 18, 21 or 24";
    return;
  }
  uint8_t out[size];
  size_t len = mnemonicToEntropy(commandData, out, sizeof(out));
  String mn = mnemonicFromEntropy(out, sizeof(out));
  deleteFile(SPIFFS, "/mn.txt");
  writeFile(SPIFFS, "/mn.txt", mn);
  printMnemonic(mn);

  message = "Restore successfull";
  subMessage = "Use `/seed` to view word list";

}

void executeSignPsbt(String commandData) {
  if (authenticated == false) {
    message = "Enter password!";
    subMessage = "8 numbers/letters";
    return;
  }
  message = "PSBT";
  subMessage = commandData;

}

void executeUnknown(String commandData) {
  message = "Unknown command";
  subMessage = "`/help` for details";
}


bool loadFiles() {
  FileData mnFile = readFile(SPIFFS, "/mn.txt");
  encrytptedSeed = mnFile.data;

  FileData pwdFile = readFile(SPIFFS, "/hash.txt");
  passwordHash = pwdFile.data;

  return mnFile.success && pwdFile.success;
}

bool wipeHww(String password) {
  if (isAlphaNumeric(password) == false)
    return false;

  deleteFile(SPIFFS, "/mn.txt");
  deleteFile(SPIFFS, "/hash.txt");
  createMn();
  keyHash = hashPassword(password); // todo: rename var
  Serial.println("wipeHww mnemonic: " + mnemonic);
  Serial.println("wipeHww keyHash: " + keyHash);
  writeFile(SPIFFS, "/mn.txt", mnemonic);
  writeFile(SPIFFS, "/hash.txt", keyHash);

  delay(DELAY_MS);
  return true;
}