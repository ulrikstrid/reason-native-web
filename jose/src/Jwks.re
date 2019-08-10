type t = {keys: list(Jwk.t)};

let to_json: t => Yojson.Basic.t =
  t => {
    let keys_json = List.map(Jwk.to_json, t.keys);
    `Assoc([("keys", `List(keys_json))]);
  };

let from_json = json => {
  keys:
    json
    |> Yojson.Basic.Util.member("keys")
    |> Yojson.Basic.Util.to_list
    |> List.map(Jwk.from_json),
};

let from_string = str => Yojson.Basic.from_string(str) |> from_json;
