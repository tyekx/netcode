<?php

session_start();

require_once 'Input.php';
require_once 'Cookie.php';
require_once 'Crypt.php';
require_once 'Medoo.php';
require_once 'Route.php';
require_once 'Session.php';
require_once 'User.php';
require_once 'ServerInstances.php';

const COOKIE_DURATION = 604800;

$db = new \Medoo\Medoo([
    'database_type' => 'mysql',
    'database_name' => 'netcode',
    'server' => '127.0.0.1',
    'username' => 'admin',
    'password' => 'password',
    'port' => 3306
]);

function AuthenticateUser($db) {
    if(Cookie::Has('netcode-auth')) {
        $token = Cookie::Get('netcode-auth');
        $ctime = time();

        $r = $db->select("users",
            ["[><]sessions" => ["id" => "user_id"]],
            ["users.id(uid)", "users.name", "users.is_banned", "sessions.id(sid)"],
            ["sessions.hash" => $token, "expires_at[>]" => $ctime]
        );

        if(count($r) != 1) {
            Cookie::Set('netcode-auth', '', -2000000);
            return null;
        }

        $updatedTime = $ctime + COOKIE_DURATION;

        $db->update("sessions", ["sessions.expires_at" => $updatedTime], ["sessions.id" => $r[0]['sid']]);

        return new User($r[0]['name'], $r[0]['uid'], $r[0]['is_banned']);
    }
    return null;
}

$user = AuthenticateUser($db);


Route::Get('/api/status', function() use ($user) {
    if($user == null) {
        return Response::EmptyJSON();
    }
    return Response::JSON($user->ToArray());
});

Route::Get('/api/servers-status', function() use ($db, $user) {
    $json = ServerInstances::GetStatus();

    if($json === FALSE) {
        return Response::JSON(["error" => "Could not reach netcode-shell"]);
    }

    return Response::RawJSON($json);
});

Route::Post('/api/register', function() use ($db, $user) {
    if($user != null) {
        Response::JSON(["error" => "You are already logged in"]);
        return;
    }

    JsonPost::Read();
    
    if(!JsonPost::Has('username')) {
        Response::JSON(["error" => "Username was not specified"]);
        return;
    }

    if(!JsonPost::Has('password')) {
        Response::JSON(["error" => "Password was not specified"]);
        return;
    }

    if(!JsonPost::Has('password_again')) {
        Response::JSON(["error" => "Password was not specified"]);
        return;
    }

    $username = JsonPost::Get('username');
    $password = JsonPost::Get('password');
    $passwordAgain = JsonPost::Get('password_again');

    if(strlen($username) > 16 || strlen($username) < 3) {
        Response::JSON(["error" => "Username must be between 3-16 characters long"]);
        return;
    }

    if(strlen($password) < 6) {
        Response::JSON(["error" => "I mean, I know, just please do at least 6 characters :D"]);
        return;
    }

    if(strlen($password) > 16) {
        Response::JSON(["error" => "Ha! Testing me, I knew it. Do less than 16 characters :P"]);
        return;
    }

    if($password != $passwordAgain) {
        Response::JSON(["error" => "Passwords does not match"]);
        return;
    }

    if(preg_match("/[a-zA-Z].*/", $username) == 0) {
        Response::JSON(["error" => "YoUr NaMe ShOuLd StArT WiTh A ChArAcTeR, NoT wItH a NuMbEr"]);
        return;
    }

    if(preg_match("/[a-zA-Z][a-zA-Z0-9]+/", $username) == 0) {
        Response::JSON(["error" => "Do not try to SQL inject me please, just go with a regular name with only english alphabet and numbers"]);
        return;
    }

    if(preg_match("/[a-zA-Z0-9]+/", $password) == 0) {
        Response::JSON(["error" => "Passwords should contain only characters from the english alphabet or numbers"]);
        return;
    }

    $query = $db->select("users", ["id"], ["name" => $username]);

    if(count($query) != 0) {
        Response::JSON(["error" => "Username is already taken, text us if you wanna claim yours"]);
        return;
    }

    $salt = Crypt::CreateRandomToken();

    $saltedPw = Crypt::CreateSaltedPassword($password, $salt);

    $db->insert("users", ["name" => $username, "password" => $saltedPw, "salt" => $salt]);

    Response::EmptyJSON();
});

Route::Post('/api/login', function() use ($db, $user) {
    if($user != null) {
        Response::JSON(["error" => "You are already logged in"]);
        return;
    }

    JsonPost::Read();

    if(!JsonPost::Has('username')) {
        Response::JSON(["error" => "Username was not specified"]);
        return;
    }

    if(!JsonPost::Has('password')) {
        Response::JSON(["error" => "Password was not specified"]);
        return;
    }

    $username = JsonPost::Get('username');
    $rawPassword = JsonPost::Get('password');

    $result = $db->select('users', ["id", "salt", "is_banned"], ["name" => $username]);

    $id; $salt; $isBanned;
    if(count($result) == 1) {
        $id = $result[0]['id'];
        $salt = $result[0]['salt'];
        $isBanned = $result[0]['is_banned'];
    } else {
        Response::JSON(["error" => "Invalid login details"]);
        return;
    }

    $saltedPassword = Crypt::CreateSaltedPassword($rawPassword, $salt);

    $user = $db->select('users', ["id", "name", "is_banned"], ["name" => $username, "password" => $saltedPassword, "id" => $id]);

    if(count($user) == 1) {
        $token = null;

        $existingSession = $db->select("sessions", ["id", "hash"], ["user_id" => $id, "expires_at[>]" => time()]);

        if(count($existingSession) >= 1) {
            $token = $existingSession[0]["hash"];
            $sessionId = $existingSession[0]["id"];

            $db->update("sessions", ["expires_at" => time() + COOKIE_DURATION], ["id" => $sessionId]);
        } else {
            $token = Crypt::CreateRandomToken();

            $db->insert("sessions", ["user_id" => $id, "hash" => $token, "expires_at" => time() + COOKIE_DURATION]);
        }

        Cookie::Set('netcode-auth', $token, COOKIE_DURATION);

        Response::JSON(["id" => $id, "name" => $username, "is_banned" => $isBanned]);
    } else {
        Response::JSON(["error" => "Invalid login details"]);
    }
});

Route::Get('/api/logout', function() use ($db, $user) {
    if($user == null) {
        Response::JSON(["error" => "You are not logged in"]);
        return;
    }
    $db->update('users', ['session' => null, 'expires_at' => 0], ["id" => $user->id]);
    Cookie::Set('netcode-auth', '', -2000000);
});

Route::Post('/api/create-session', function() use ($db, $user) {
    if($user == null) {
        Response::JSON(["error" => "You are not logged in"]);
        return;
    }

    JsonPost::Read();

    if(!JsonPost::Has('max_players')) {
        Response::JSON(["error" => "You must specify the maximum number of players"]);
        return;
    }

    $maxPlayers = intval(JsonPost::Get('max_players'));
    $port = intval(JsonPost::Get('port'));
    $interval = intval(JsonPost::Get('interval'));

    if($maxPlayers < 2 || $maxPlayers > 16) {
        Response::JSON(["error" => "Max players must be between 2-16"]);
        return;
    }

    if($interval < 1 || $interval > 1000) {
        Response::JSON(["error" => "Interval must be between 1-1000"]);
        return;
    }

    $json = ServerInstances::CreateServer($maxPlayers, $port, $user->id, $interval);

    if($json === FALSE) {
        return Response::JSON(["error" => "Could not reach netcode-shell"]);
    }

    $arr = json_decode($json, true);

    if(!array_key_exists("Port", $arr)) {
        return Response::JSON(["error" => "Unexpected error"]);
    }

    Response::RawJSON($json);
});

Route::Get('/api/list-sessions', function() use ($db) {
   $r = $db->select("game_servers",
    [ "[>]game_sessions" => ["id" => "game_server_id"], "[><]users" => ["owner_id" => "id"]],
    [
        "game_servers.hostname",
        "game_servers.server_ip",
        "game_servers.control_port",
        "game_servers.game_port",
        "game_servers.max_players",
        "game_servers.id",
        "users.name(owner)",
        "active_players" => \Medoo\Medoo::raw('SUM(IF(<game_sessions.left_at> IS NULL AND <game_sessions.user_id> IS NOT NULL, 1, 0))')
        /*
        sum those that are null because they have not left the server yet, and not because they got introduced by left join
        for future me: cant use where clause because it semantically creates an inner join instead
        */
    ],
    [
        "game_servers.status" => 1,
        "GROUP" => [
            "game_servers.id"
        ]
    ]);
   Response::Json($r);
});

Route::Run();

?>
