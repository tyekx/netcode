<?php

class JsonPost {
    private static $data;

    public static function Read() {
        self::$data = json_decode(file_get_contents('php://input'), true);
    }

    public static function Raw() {
        return self::$data;
    }

    public static function Has($key) {
        return array_key_exists($key, self::$data);
    }

    public static function Get($key) {
        if(self::Has($key)) {
            return self::$data[$key];
        }
        return null;
    }
}

class PostData {
    public static function Has($key) {
        return array_key_exists($key, $_POST);
    }

    public static function Get($key) {
        if(self::Has($key)) {
            return $_POST[$key];
        }
        return null;
    }
}

class GetData {
    public static function Has($key) {
        return array_key_exists($key, $_GET);
    }

    public static function Get($key) {
        if(self::Has($key)) {
            return $_GET[$key];
        }
        return null;
    }
}
