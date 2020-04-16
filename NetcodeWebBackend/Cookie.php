<?php

class Cookie {

    public static function Has($key) {
        return array_key_exists($key, $_COOKIE);
    }

    public static function Get($key) {
        if(self::Has($key)) {
            return $_COOKIE[$key];
        }
        return null;
    }

    public static function Set($key, $value, $expire = 0) {
        if($expire != 0) {
            $expire = time() + $expire;
        }
        setcookie($key, $value, $expire);
    }

    public static function Delete($key) {
        setcookie($key, "", 1);
    }

}
