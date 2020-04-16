<?php

class Session {
    public static function Has($key) {
        return array_key_exists($key, $_SESSION);
    }

    public static function Get($key) {
        if(self::Has($key)) {
            return $_SESSION[$key];
        }
        return null;
    }

    public static function Delete($key) {
        unset($_SESSION[$key]);
    }

    public static function Set($key, $value) {
        $_SESSION[$key] = $value;
    }
}
