/**
 * rsa_dec.h
 * 
 * Header for RSA decryption function declarations 
 * 
 * Author Dalton Kinney
 * Created Feb 4th, 2024
*/
#include <stdio.h>
#include <stdlib.h>

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include "rsa_dec.h"

#define MAX_BUF 2048
//#define DEBUG 1

static RSA* read_privkey(char* filename) { 
    // Open the private key pem file to be read
    FILE* f = fopen(filename, "rb");
    if (!f) {
        perror("Error opening private key file");
        return NULL;
    }

    // Read the base64 decoded PEM file into the RSA structure
    RSA* rsa_private_key = PEM_read_RSAPrivateKey(f, NULL, NULL, NULL);

    if (!rsa_private_key) {
        perror("Unable to get private key from pem file");
        fclose(f);
        return NULL;
    }
    fclose(f);

#ifdef DEBUG
    // Print the Private key mod and expo
    const BIGNUM *n, *e;
    RSA_get0_key(rsa_private_key, &n, &e, NULL);

    char *n_str = BN_bn2hex(n);
    char *e_str = BN_bn2hex(e);

    printf("Private key modulus (n):\n%s\n", n_str);
    printf("Private key exponent (e):\n%s\n", e_str);

    OPENSSL_free(n_str);
    OPENSSL_free(e_str);
#endif

    return rsa_private_key;
}

int rsa_dec(unsigned char* data, int data_len, char* priv_key) { 
    unsigned char dec_data[MAX_BUF] = {0}; // Initialize buffer to hold out decrypted data

    // Print data read in  
#ifdef DEBUG
    printf("Data read:\nlen %d\n", data_len);
    for (int i = 0; i < data_len; ++i) {
        printf("%02x", data[i]);
    }
    printf("\n");
#endif

    // Read the private key from the PEM file 
    RSA *priv = read_privkey(priv_key);
    if (!priv) { 
        printf("Unable to read private key key\n");
        return 0;
    }

    // Decrypt data with the private key
    int dec_len = RSA_private_decrypt(data_len, data, dec_data, priv, RSA_PKCS1_PADDING);
    if (dec_len == -1) { 
        unsigned long err = ERR_get_error(); // Get the error code
        printf("RSA_private decrypt failed with error code: %s\n", ERR_reason_error_string(err));
        RSA_free(priv);
        return 0;
    }

    // Overwrite buffer with encrypted data, with decrypted data
    strcpy(data, dec_data);

    RSA_free(priv);
    return data;
}