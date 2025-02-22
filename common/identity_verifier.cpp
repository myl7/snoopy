// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <openenclave/enclave.h>
#include <stdlib.h>
#include <string>
#include "cert_verify_config.h"
#include "utility.h"

oe_result_t enclave_claims_verifier_callback_server(oe_claim_t *claims, size_t claims_length, void *arg) {
  return enclave_claims_verifier_callback(claims, claims_length, arg, SERVER_PUBKEY);
}

oe_result_t enclave_claims_verifier_callback_client(oe_claim_t *claims, size_t claims_length, void *arg) {
  return enclave_claims_verifier_callback(claims, claims_length, arg, CLIENT_PUBKEY);
}

oe_result_t enclave_claims_verifier_callback(oe_claim_t *claims, size_t claims_length, void *arg, char pubkey[]) {
  OE_UNUSED(arg);

  oe_result_t result = OE_VERIFY_FAILED;
  const oe_claim_t *claim;

  printf(TLS_ENCLAVE
    "enclave_claims_verifier_callback is called with enclave "
    "identity information extracted from the evidence claims:\n");

  // Enclave's security version
  if ((claim = find_claim(claims, claims_length, OE_CLAIM_SECURITY_VERSION)) == nullptr) {
    printf(TLS_ENCLAVE "could not find OE_CLAIM_SECURITY_VERSION\n");
    goto exit;
  }
  if (claim->value_size != sizeof(uint32_t)) {
    printf(TLS_ENCLAVE "security_version size(%lu) checking failed\n", claim->value_size);
    goto exit;
  }
  printf(TLS_ENCLAVE "\nsecurity_version = %d\n", *claim->value);

  // The unique ID for the enclave, for SGX enclaves, this is the MRENCLAVE
  // value
  if ((claim = find_claim(claims, claims_length, OE_CLAIM_UNIQUE_ID)) == nullptr) {
    printf(TLS_ENCLAVE "could not find OE_CLAIM_UNIQUE_ID\n");
    goto exit;
  }
  if (claim->value_size != OE_UNIQUE_ID_SIZE) {
    printf(TLS_ENCLAVE "unique_id size(%lu) checking failed\n", claim->value_size);
    goto exit;
  }

  if (verify_claim_value(claim) != OE_OK) {
    printf(TLS_ENCLAVE "failed: unique_id not equal\n");
    goto exit;
  }

  // The signer ID for the enclave, for SGX enclaves, this is the MRSIGNER
  // value
  if ((claim = find_claim(claims, claims_length, OE_CLAIM_SIGNER_ID)) == nullptr) {
    printf(TLS_ENCLAVE "could not find OE_CLAIM_SIGNER_ID\n");
    goto exit;
  }
  if (claim->value_size != OE_SIGNER_ID_SIZE) {
    printf(TLS_ENCLAVE "signer_id size(%lu) checking failed\n", claim->value_size);
    goto exit;
  }
  printf(TLS_ENCLAVE "\nverify signer_id:\n");
  for (size_t i = 0; i < claim->value_size; i++) printf("0x%0x ", (uint8_t)claim->value[i]);

  if (!verify_signer_id((char *)pubkey, sizeof(SERVER_PUBKEY), claim->value, claim->value_size)) {
    printf(TLS_ENCLAVE "failed: signer_id not equal\n");
    goto exit;
  }
  printf(TLS_ENCLAVE "signer_id validation passed\n");

  // The product ID for the enclave, for SGX enclaves, this is the ISVPRODID
  // value
  if ((claim = find_claim(claims, claims_length, OE_CLAIM_PRODUCT_ID)) == nullptr) {
    printf(TLS_ENCLAVE "could not find OE_CLAIM_PRODUCT_ID\n");
    goto exit;
  }
  if (claim->value_size != OE_PRODUCT_ID_SIZE) {
    printf(TLS_ENCLAVE "product_id size(%lu) checking failed\n", claim->value_size);
    goto exit;
  }
  printf(TLS_ENCLAVE "\nproduct_id:\n");
  for (size_t i = 0; i < claim->value_size; i++) printf("0x%0x ", (uint8_t)claim->value[i]);
  printf("\n\n");

  result = OE_OK;
exit:
  return result;
}
