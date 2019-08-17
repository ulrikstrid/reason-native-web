# Reason Native Web

A toolkit for building web applications in reason native

## Needed resolutions:

```json
  "resolutions": {
    "@opam/cstruct": "3.3.0",
    "@opam/httpaf-lwt-unix": "anmonteiro/httpaf:httpaf-lwt-unix.opam#5dff1b4",
    "@opam/httpaf-lwt": "anmonteiro/httpaf:httpaf-lwt.opam#5dff1b4",
    "@opam/httpaf": "anmonteiro/httpaf:httpaf.opam#5dff1b4",
    "@opam/nocrypto": "mirleft/ocaml-nocrypto:opam#ed7bb8d",
    "@esy-packages/esy-openssl": "esy-packages/esy-openssl#f731e9c",
    "@opam/ssl": {
      "source": "savonet/ocaml-ssl:ssl.opam#9dd1cbf71839195e115b8e2438554c530e0f0ed0",
      "override": {
        "dependencies": {
          "@opam/conf-openssl": "*",
          "@esy-packages/esy-openssl": "esy-packages/esy-openssl#f731e9c"
        },
        "buildEnv": {
          "CFLAGS": "-I$OPENSSL_INCLUDE_PATH",
          "LDFLAGS": "-L$OPENSSL_LIB_PATH"
        }
      }
    },
    "@opam/conf-openssl": {
      "source": "no-source:",
      "override": {
        "build": ["pkg-config openssl"],
        "dependencies": {
          "@opam/conf-pkg-config": "1.1",
          "@esy-packages/esy-openssl": "esy-packages/esy-openssl#f731e9c"
        },
        "buildEnv": {
          "CFLAGS": "-I$OPENSSL_INCLUDE_PATH $CFLAGS",
          "LDFLAGS": "-L$OPENSSL_LIB_PATH $LDFLAGS"
        }
      }
    }
  }
```
