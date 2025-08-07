#!/bin/bash

# Script to generate self-signed SSL certificates for HTTPS server

CERT_DIR="./certs"
CERT_FILE="$CERT_DIR/server.crt"
KEY_FILE="$CERT_DIR/server.key"
CONFIG_FILE="$CERT_DIR/server.conf"

# Create certificates directory
mkdir -p "$CERT_DIR"

# Create OpenSSL configuration file for self-signed certificate
cat > "$CONFIG_FILE" << EOF
[req]
default_bits = 2048
prompt = no
distinguished_name = dn
req_extensions = v3_req

[dn]
C=US
ST=State
L=City
O=Organization
OU=Department
CN=localhost

[v3_req]
basicConstraints = CA:FALSE
keyUsage = nonRepudiation, digitalSignature, keyEncipherment
subjectAltName = @alt_names

[alt_names]
DNS.1 = localhost
DNS.2 = *.localhost
IP.1 = 127.0.0.1
IP.2 = ::1
EOF

echo "Generating self-signed SSL certificate..."

# Generate private key and certificate
openssl req -new -x509 -days 365 -nodes \
    -keyout "$KEY_FILE" \
    -out "$CERT_FILE" \
    -config "$CONFIG_FILE" \
    -extensions v3_req

if [ $? -eq 0 ]; then
    echo "✅ SSL certificate generated successfully!"
    echo "📁 Certificate: $CERT_FILE"
    echo "🔑 Private key: $KEY_FILE"
    echo ""
    echo "Certificate details:"
    openssl x509 -in "$CERT_FILE" -text -noout | grep -A 1 "Subject:"
    echo ""
    echo "⚠️  This is a self-signed certificate. Browsers will show a security warning."
    echo "    Click 'Advanced' → 'Proceed to localhost (unsafe)' to continue."
else
    echo "❌ Failed to generate SSL certificate"
    exit 1
fi
