type scope =
  | OpenID
  | Profile
  | Email
  | Address
  | Phone
  | NonStandard(string);

let string_to_scope =
  fun
  | "openid" => OpenID
  | "profile" => Profile
  | "email" => Email
  | "address" => Address
  | "phone" => Phone
  | non_standard => NonStandard(non_standard);

let scope_to_claims = scope => {
  switch (scope) {
  | OpenID => []
  | Profile => [
      Claims.NonEssential("name"),
      Claims.NonEssential("family_name"),
      Claims.NonEssential("given_name"),
      Claims.NonEssential("middle_name"),
      Claims.NonEssential("nickname"),
      Claims.NonEssential("preferred_username"),
      Claims.NonEssential("profile"),
      Claims.NonEssential("picture"),
      Claims.NonEssential("website"),
      Claims.NonEssential("gender"),
      Claims.NonEssential("birthdate"),
      Claims.NonEssential("zoneinfo"),
      Claims.NonEssential("locale"),
      Claims.NonEssential("updated_at"),
    ]
  | Email => [
      Claims.NonEssential("email"),
      Claims.NonEssential("email_verified"),
    ]
  | Address => [Claims.NonEssential("address")]
  | Phone => [
      Claims.NonEssential("phone_number"),
      Claims.NonEssential("phone_number_verified"),
    ]
  | NonStandard(s) => [Claims.Essential(s)]
  };
};
