<?php

class User {
    public $name;
    public $id;
    public $isBanned;

    public function __construct($n, $i, $b) {
        $this->name = $n;
        $this->id = $i;
        $this->isBanned = $b;
    }

    public function ToArray() {
        return [
            "name" => $this->name,
            "id" => $this->id,
            "is_banned" => $this->isBanned
        ];
    }


}
