<?php

class Response {

    public static function JSON($assoc, $code = 200) {
        header("Content-Type: application/json", true, $code);
        echo json_encode($assoc);
    }

    public static function EmptyJSON($code = 200) {
        header("Content-Type: application/json", true, $code);
        echo "{}";
    }

    public static function RawJSON($json, $code = 200) {
        header("Content-Type: application/json", true, $code);
        echo $json;
    }

}
