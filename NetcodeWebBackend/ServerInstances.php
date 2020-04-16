<?php

class ServerInstances {
    public static function CreateServer($maxPlayers, $portHint, $ownerId, $interval) {
        $url = "http://127.0.0.1:4555";
        $data = ["max_players" => (int)$maxPlayers, "owner_id" => (int)$ownerId, "port" => (int)$portHint, "interval" => (int)$interval];

        $options = [
            'http' => [
                'header' => 'Content-Type: Application/json',
                'method' => 'POST',
                'content' => json_encode($data)
            ]
        ];

        $context = stream_context_create($options);
        return @file_get_contents($url, false, $context);
    }

    public static function GetStatus() {
        $url= "http://127.0.0.1:4555/get-status";

        return @file_get_contents($url);
    }
}
