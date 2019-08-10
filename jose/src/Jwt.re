type algorithm = [ | `RSA | `Unknown];
type typ = [ | `JWT | `Unknown];

type header = {
  alg: algorithm,
  typ,
  kid: string,
};

let make_header = (jwk: Jwk.t) => {alg: `RSA, typ: `JWT, kid: jwk.kid};

let base64_url_encode =
  Base64.encode(~pad=false, ~alphabet=Base64.uri_safe_alphabet);
let base64_url_decode =
  Base64.decode(~pad=false, ~alphabet=Base64.uri_safe_alphabet);

let header_to_string = header => {
  let alg =
    switch (header.alg) {
    | `RSA => "RS256"
    | _ => "Unknown"
    };

  let typ =
    switch (header.typ) {
    | `JWT => "JWT"
    | _ => "Unknown"
    };

  `Assoc([
    ("alg", `String(alg)),
    ("typ", `String(typ)),
    ("kid", `String(header.kid)),
  ])
  |> Yojson.Basic.to_string
  |> base64_url_encode;
};

let string_to_header = header_str => {
  let to_alg = alg =>
    switch (alg) {
    | "RS256" => `RSA
    | _ => `Unknown
    };

  let to_typ = typ =>
    switch (typ) {
    | "JWT" => `JWT
    | _ => `Unknown
    };

  base64_url_decode(header_str)
  |> CCResult.map(decoded_header => {
       Yojson.Basic.from_string(decoded_header)
       |> (
         json => {
           alg:
             Yojson.Basic.Util.member("alg", json)
             |> Yojson.Basic.Util.to_string
             |> to_alg,
           typ:
             Yojson.Basic.Util.member("typ", json)
             |> Yojson.Basic.Util.to_string
             |> to_typ,
           kid:
             Yojson.Basic.Util.member("kid", json)
             |> Yojson.Basic.Util.to_string,
         }
       )
     });
};

type payload = Yojson.Basic.t;
type claim = (string, Yojson.Basic.t);

let empty_payload = `Assoc([]);

let payload_to_string = payload => {
  payload |> Yojson.Basic.to_string |> base64_url_encode;
};

let string_to_payload = payload_str => {
  base64_url_decode(payload_str) |> CCResult.map(Yojson.Basic.from_string);
};

type signature = string;

type t = {
  header,
  payload,
  header_str: string,
  payload_str: string,
  signature,
};

let add_claim =
    (claim_name: string, claim_value: Yojson.Basic.t, payload: payload) =>
  `Assoc([
    (claim_name, claim_value),
    ...Yojson.Basic.Util.to_assoc(payload),
  ]);

let sign = (header, key, payload) => {
  let header_str = header_to_string(header);
  let payload_str = payload_to_string(payload);

  CCResult.both(header_str, payload_str)
  |> CCResult.map(((header_str, payload_str)) => {
       let input_str = header_str ++ "." ++ payload_str;

       let signature =
         `Message(Cstruct.of_string(input_str))
         |> Nocrypto.Rsa.PKCS1.sign(~hash=`SHA256, ~key)
         |> Cstruct.to_string;

       {header, header_str, payload, payload_str, signature};
     });
};

let to_string = t =>
  t.header_str ++ "." ++ t.payload_str ++ "." ++ t.signature;

let from_string = token => {
  CCString.split(~by=".", token)
  |> (
    fun
    | [header_str, payload_str, signature] => {
        let header = string_to_header(header_str);
        let payload = string_to_payload(payload_str);
        CCResult.both(header, payload)
        |> CCResult.flat_map(((header, payload)) =>
             Ok({header, payload, header_str, payload_str, signature})
           );
      }
    | _ => Error(`Msg("token didn't include header, payload or signature"))
  );
};

let sha256_asn1 =
  Cstruct.of_string(
    "\x30\x31\x30\x0d\x06\x09\x60\x86\x48\x01\x65\x03\x04\x02\x01\x05\x00\x04\x20",
  );

let pkcs1_sig = (asn1, body) => {
  let len = Cstruct.len(asn1);
  if (Cstruct.check_bounds(body, len)) {
    switch (Cstruct.split(~start=0, body, len)) {
    | (a, b) when Cstruct.equal(a, asn1) => Some(b)
    | _ => None
    };
  } else {
    None;
  };
};

let verify_internal = (~n, ~e, t) => {
  open Nocrypto.Numeric;

  let signature = Cstruct.of_string(t.signature);
  /* TODO: Move to Jwk module */
  let n = n |> Cstruct.of_string |> Z.of_cstruct_be;
  let e = e |> Cstruct.of_string |> Z.of_cstruct_be;

  switch (Nocrypto.Rsa.PKCS1.sig_decode(~key={n, e}, signature)) {
  | None => Error(`Msg("Could not decode signature"))
  | Some(message) =>
    switch (pkcs1_sig(sha256_asn1, message)) {
    | None => Error(`Msg("PKCS something something, I must understand this"))
    | Some(decrypted_sign) =>
      let token_hash =
        t.header_str
        ++ "."
        ++ t.payload_str
        |> Cstruct.of_string
        |> Nocrypto.Hash.SHA256.digest;
      Ok(Cstruct.equal(decrypted_sign, token_hash));
    }
  };
};

let verify = (~jwks: list(Jwk.t), t) => {
  let header = t.header;
  let payload = t.payload;

  (
    switch (header.alg) {
    | `RSA => Ok(header.alg)
    | _ => Error(`Msg("alg must be RS256"))
    }
  )
  |> CCResult.flat_map(_ =>
       switch (header.typ) {
       | `JWT => Ok(header.typ)
       | _ => Error(`Msg("typ must be JWT"))
       }
     )
  |> CCResult.flat_map(_ =>
       CCList.find_opt((jwk: Jwk.t) => jwk.kid == header.kid, jwks)
       |> (
         fun
         | Some(jwk) => Ok(jwk)
         | None => Error(`Msg("Did not find key with correct kid"))
       )
     )
  |> CCResult.flat_map((jwk: Jwk.t) =>
       CCResult.both(base64_url_decode(jwk.n), base64_url_decode(jwk.e))
     )
  |> CCResult.flat_map(((n, e)) => verify_internal(~n, ~e, t))
  |> CCResult.flat_map(_ => {
       module Json = Yojson.Basic.Util;
       switch (Json.member("exp", payload) |> Json.to_int_option) {
       | Some(exp) when exp > int_of_float(Unix.time()) => Ok(t)
       | Some(_exp) => Error(`Msg("Token expired"))
       | None => Error(`Msg({|No "exp" key found|}))
       };
     });
};
