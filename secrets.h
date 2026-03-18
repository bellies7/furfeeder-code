#ifndef SECRETS_H
#define SECRETS_H

#include <pgmspace.h>

// ============================================================
// STEP 1: Enter your AWS IoT Thing name
// This is the name you gave your Thing in AWS IoT Core
// ============================================================
#define THINGNAME "YOUR_THING_NAME"

// ============================================================
// STEP 2: Enter your WiFi credentials
// ============================================================
const char WIFI_SSID[]     = "YOUR_WIFI_SSID";
const char WIFI_PASSWORD[] = "YOUR_WIFI_PASSWORD";

// ============================================================
// STEP 3: Enter your AWS IoT endpoint
// Found in AWS IoT Console → Settings → Domain Configuration
// Looks like: xxxx.iot.ap-southeast-1.amazonaws.com
// ============================================================
const char AWS_IOT_ENDPOINT[] = "YOUR_AWS_IOT_ENDPOINT.amazonaws.com";

// ============================================================
// STEP 4: Paste your Amazon Root CA 1 certificate below
// From the file: AmazonRootCA1.pem
// ============================================================
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
PASTE YOUR AMAZON ROOT CA 1 CERTIFICATE HERE
-----END CERTIFICATE-----
)EOF";

// ============================================================
// STEP 5: Paste your Device Certificate below
// From the file: xxxx-certificate.pem.crt
// ============================================================
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
PASTE YOUR DEVICE CERTIFICATE HERE
-----END CERTIFICATE-----
)KEY";

// ============================================================
// STEP 6: Paste your Device Private Key below
// From the file: xxxx-private.pem.key
// ============================================================
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
PASTE YOUR PRIVATE KEY HERE
-----END RSA PRIVATE KEY-----
)KEY";

#endif // SECRETS_H
