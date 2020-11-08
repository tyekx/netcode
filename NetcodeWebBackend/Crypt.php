<?php

class Crypt {
    public static function CreateRandomToken() {
        return bin2hex(random_bytes(32));
    }

    public static function CreateSaltedPassword($rawPassword) {
        return password_hash($rawPassword, PASSWORD_BCRYPT);
    }

    public static function VerifyPassword($rawPassword, $hash) {
        return password_verify($rawPassword, $hash);
    }
}
