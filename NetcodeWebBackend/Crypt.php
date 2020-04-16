<?php

class Crypt {
    public static function CreateRandomToken() {
        return hash("sha256", rand() . uniqid() . rand());
    }


    public static function CreateSaltedPassword($rawPassword, $salt) {
        return hash("sha256", $rawPassword . $salt);
    }
}
