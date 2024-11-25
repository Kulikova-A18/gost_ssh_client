#!/bin/bash

SCRIPT_DIR=$(dirname "$(readlink -f "$0")")

# Specify the path to the library
MODULE_PATH="/usr/lib64/librtpkcs11ecp.so"
NAME_SERVER="redos-server"
NAME_SERVER_IP="10.0.2.15"
NAME_SERVER_ALG_SSH="grasshopper-cbc@altlinux.org"

echo "INFO: Verify the existence of certificates and public/private key pairs for the Rutoken (open token)"
if ! pkcs11-tool --module "$MODULE_PATH" -T; then
    echo "ERROR: The verification process encountered an error due to the unavailability of certificates and key pairs specific to Rutoken (open token)"
    exit 1
fi

echo "INFO: Verify the existence of certificates and public/private key pairs for the Rutoken"
if ! pkcs11-tool --module "$MODULE_PATH" -O; then
    echo "ERROR: The verification process encountered an error due to the unavailability of certificates and key pairs specific to Rutoken"
    exit 1
fi

echo "INFO: Get all IDs"
ids=$(pkcs11-tool --module "$MODULE_PATH" -O | grep -oP 'ID:s*K.*')

echo "INFO: Verify the presence of any IDs"
if [ -z "$ids" ]; then
    echo "ERROR: No IDs found"
    exit 1
fi

# Process each ID
while IFS= read -r id; do
    echo "INFO: Processing ID: $id"

    # Form the filename based on ID
    crt_file_name="cert_${id}.crt"
    pem_file_name="cert_${id}.pem"
    
    echo "INFO: Duplicating the certificate into a file"
    pkcs11-tool --module "$MODULE_PATH" -r -y cert --id "$id" > "./$crt_file_name"
    
    # Check if the operation was successful
    if [ $? -eq 0 ]; then
        echo "INFO: Certificate for ID $id saved to file $crt_file_name"
        
        # Convert to PEM format
        openssl x509 -inform DER -in "./$crt_file_name" -out "./$pem_file_name"
        
        if [ $? -eq 0 ]; then
            echo "INFO: Certificate successfully converted to PEM format and saved to file $pem_file_name"
            ./ssh_client $NAME_SERVER_IP $NAME_SERVER $NAME_SERVER_ALG_SSH /tmp/ssh_control $pem_file_name
        else
            echo "ERROR: Error converting certificate to PEM format for ID $id"
        fi

        rm "./$crt_file_name" || echo "WARNING: Failed to delete the temporary CRT file"
    else
        echo "ERROR: Error saving certificate for ID $id"
    fi

done <<< "$ids"
