function verifyTest() {
  var result = 1.2718440192507248;
  var expectedResult = 1.2718440192507248;
  if (result !== expectedResult) {
    return false;
  } else {
    return true;
  }
}

if (verifyTest()) {
  print("passed");
}
