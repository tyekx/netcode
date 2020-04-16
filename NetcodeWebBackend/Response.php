<?php

class Response {

    public static function JSON($assoc) {
        header("Content-Type: application/json");
        echo json_encode($assoc);
    }

    public static function EmptyJSON() {
        header("Content-Type: application/json");
        echo "{}";
    }

    public static function RawJSON($json) {
        header("Content-Type: application/json");
        echo $json;
    }

}
