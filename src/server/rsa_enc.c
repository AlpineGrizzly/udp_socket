/**
 * rsa_enc.c
 * 
 * implementation file for function definitions of RSA encryption 
 * 
 * Author Dalton Kinney
 * Created Feb 4th, 20234
*/
#include <stdio.h>
#include <stdlib.h>

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include "rsa_enc.h"

#define MAX_BUF 2048
//#define DEBUG 1

static RSA* read_pubkey(char* filename) {     
    // Open the public key pem file to be read
    FILE* f = fopen(filename, "rb");
    if (!f) {
        perror("Error opening public key file");
        return NULL;
    }

    // Read the base64 decoded PEM file into the RSA structure
    RSA* rsa_public_key = PEM_read_RSA_PUBKEY(f, NULL, NULL, NULL);
    if (!rsa_public_key) {
        perror("Unable to get public key from pem file");
        fclose(f);
        return NULL;
    }
    fclose(f);

#ifdef DEBUG
    // Print the Public key mod and expo
    const BIGNUM *n, *e;
    RSA_get0_key(rsa_public_key, &n, &e, NULL);

    char *n_str = BN_bn2hex(n);
    char *e_str = BN_bn2hex(e);

    printf("Public key modulus (n):\n%s\n", n_str);
    printf("Public key exponent (e):\n%s\n", e_str);

    OPENSSL_free(n_str);
    OPENSSL_free(e_str);
#endif

    return rsa_public_key;
}

int rsa_enc(unsigned char* data, int data_len, unsigned char* pub_key) {
    unsigned char enc_data[MAX_BUF] = {0}; // Buffer to hold encrypted data

    // Print data read in  
    printf("Data read:\nlen %d\n", data_len);
    for (int i = 0; i < data_len; ++i) {
        printf("%02x", data[i]);
    }
    printf("\n");
    
#ifdef DEBUG
    // Read the public key from the PEM file
    printf("Opening %s\n", pub_key);
#endif 

    RSA *pub = read_pubkey(pub_key);
    if (!pub) { 
        printf("Unable to read %s\n", data);
        return 0;
    }

    // Encrypt the data with the public key
    int enc_len = RSA_public_encrypt(data_len, data, enc_data, pub, RSA_PKCS1_PADDING);
    if (enc_len == -1) {
        unsigned long err = ERR_get_error(); // Get the error code
        printf("RSA_public_encrypt failed with error code: %s\n", ERR_reason_error_string(err));        
        RSA_free(pub);
        return 0;
    }

    // Print encrypted text 
    printf("Encrypted message (in hexadecimal):\n");
    for (int i = 0; i < enc_len; ++i) {
        printf("%02x", enc_data[i]);
    }
    printf("\n");

    // Overwrite encrypted data with decrypted data
    strcpy(data, enc_data);

    // Free RSA structure
    RSA_free(pub);
    return 1; // Return our encrypted data
}