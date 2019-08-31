module Utils exposing
    ( decodeString
    , encodeString
    , justValues
    , last
    , makeCall
    , maybeToList
    , wrap
    , zip
    )

import Regex
import Types exposing (MalExpr(..))


decodeString : String -> String
decodeString =
    let
        unescape { match } =
            case match of
                "\\n" ->
                    "\n"

                "\\\"" ->
                    "\""

                "\\\\" ->
                    "\\"

                other ->
                    other
    in
    String.slice 1 -1
        >> Regex.replace (regex "\\\\[\\\"\\\\n]") unescape



-- helps replace all the encodes found into a string


regex : String -> Regex.Regex
regex str =
    Maybe.withDefault Regex.never <|
        Regex.fromString str


encodeString : String -> String
encodeString =
    let
        escape { match } =
            case match of
                "\n" ->
                    "\\n"

                "\"" ->
                    "\\\""

                "\\" ->
                    "\\\\"

                other ->
                    other
    in
    wrap "\"" "\""
        << Regex.replace (regex "[\\n\\\"\\\\]") escape


makeCall : String -> List MalExpr -> MalExpr
makeCall symbol args =
    MalList <| MalSymbol symbol :: args


wrap : String -> String -> String -> String
wrap prefix suffix str =
    prefix ++ str ++ suffix


maybeToList : Maybe a -> List a
maybeToList m =
    case m of
        Just x ->
            [ x ]

        Nothing ->
            []


zip : List a -> List b -> List ( a, b )
zip a b =
    case ( a, b ) of
        ( [], _ ) ->
            []

        ( _, [] ) ->
            []

        ( x :: xs, y :: ys ) ->
            ( x, y ) :: zip xs ys


last : List a -> Maybe a
last list =
    case list of
        [] ->
            Nothing

        [ x ] ->
            Just x

        x :: xs ->
            last xs


justValues : List (Maybe a) -> List a
justValues list =
    case list of
        [] ->
            []

        (Just x) :: rest ->
            x :: justValues rest

        Nothing :: rest ->
            justValues rest


flip : (a -> b -> c) -> (b -> a -> c)
flip f b a =
    f a b