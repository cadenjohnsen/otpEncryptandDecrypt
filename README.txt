compile all command:
chmod +x compileall.sh
./compileall.sh

Run test script:
./testScript.sh randomNumber1 randomNumber2 > myTestResults 2>&1

Test otp_enc/otp_enc_d connection:
otp_enc_d portNumber &
otp_enc plaintextfile keyfile portNumber

Kill past processes:
killall -q -u johnsenc otp_*
