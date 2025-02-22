// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <errno.h>
#include <mbedtls/certs.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/debug.h>
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/platform.h>
#include <mbedtls/ssl.h>
#include <string.h>
#include "cert_verify_config.h"
#include "utility.h"

oe_result_t enclave_claims_verifier_callback_server(oe_claim_t *claims, size_t claims_length, void *arg);

oe_result_t enclave_claims_verifier_callback_client(oe_claim_t *claims, size_t claims_length, void *arg);

int cert_verify_callback_server(void *data, mbedtls_x509_crt *crt, int depth, uint32_t *flags) {
  return cert_verify_callback(data, crt, depth, flags, true);
}

int cert_verify_callback_client(void *data, mbedtls_x509_crt *crt, int depth, uint32_t *flags) {
  return cert_verify_callback(data, crt, depth, flags, false);
}

// If set, the verify callback is called for each certificate in the chain.
// The verification callback is supposed to return 0 on success. Otherwise, the
// verification failed.
int cert_verify_callback(void *data, mbedtls_x509_crt *crt, int depth, uint32_t *flags, bool isServer) {
  oe_result_t result = OE_FAILURE;
  int ret = 1;
  unsigned char *cert_buf = NULL;
  size_t cert_size = 0;

  (void)data;

  printf(TLS_ENCLAVE "Received TLS certificate\n");
  printf(TLS_ENCLAVE "cert_verify_callback with depth = %d\n", depth);

  cert_buf = crt->raw.p;
  cert_size = crt->raw.len;

  printf(TLS_ENCLAVE "crt->version = %d cert_size = %zu\n", crt->version, cert_size);

  if (cert_size <= 0) goto exit;

  if (isServer) {
    result = oe_verify_attestation_certificate_with_evidence(
      cert_buf, cert_size, enclave_claims_verifier_callback_server, NULL);
  } else {
    result = oe_verify_attestation_certificate_with_evidence(
      cert_buf, cert_size, enclave_claims_verifier_callback_client, NULL);
  }
  if (result != OE_OK) {
    printf(TLS_ENCLAVE
      "oe_verify_attestation_certificate_with_evidence failed "
      "with result = %s\n",
      oe_result_str(result));
    goto exit;
  }
  ret = 0;
  *flags = 0;

exit:
  return ret;
}
