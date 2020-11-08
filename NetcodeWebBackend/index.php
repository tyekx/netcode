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

$db = null;
try {
    $db = new \Medoo\Medoo([
        'database_type' => 'mysql',
        'database_name' => 'netcode',
        'server' => '127.0.0.1',
        'username' => 'admin',
        'password' => 'password',
        'port' => 3306
    ]);
} catch(Exception $e) {
    return Response::JSON(["error" => "Failed to connect to database"], 500);
}

function AuthenticateUser($db) {
    if(Cookie::Has('netcode-auth')) {
        $token = Cookie::Get('netcode-auth');

        $r = $db->get("users",
            ["[><]sessions" => ["id" => "user_id"]],
            ["users.id(uid) [Int]", "users.name [String]", "users.is_banned [Bool]", "sessions.id(sid)"],
            ["sessions.hash" => $token, "expires_at[>]" => \Medoo\Medoo::Raw("NOW(6)")]
        );

        if(empty($r)) {
            Cookie::Set('netcode-auth', '', -2000000);
            return null;
        }

        $db->update("sessions", ["sessions.expires_at" => \Medoo\Medoo::raw("DATE_ADD(NOW(6), INTERVAL 14 DAY)")], ["sessions.id" => $r['sid']]);

        return new User($r['name'], $r['uid'], $r['is_banned']);
    }
    return null;
}

$user = AuthenticateUser($db);


Route::Get('/api/status', function() use ($user) {
    if($user == null) {
        return Response::EmptyJSON(200);
    }
    return Response::JSON($user->ToArray(), 200);
});

Route::Get('/api/latest-version', function() use ($db) {
    $resp = $db->get("versions", [
        "version_major(major) [Int]",
        "version_minor(minor) [Int]",
        "version_patch(patch) [Int]",
        "version_build(build) [Int]",
        "filepath(path) [String]",
        "hash_sha1 [String]" => \Medoo\Medoo::raw("HEX(<hash_sha1>)"),
        "hash_sha256 [String]" => \Medoo\Medoo::raw("HEX(<hash_sha256>)"),
        "hash_sha512 [String]" => \Medoo\Medoo::raw("HEX(<hash_sha512>)"),
        "hash_md5 [String]" => \Medoo\Medoo::raw("HEX(<hash_md5>)")
    ], [
        "ORDER" => [
            "version_major" => "DESC",
            "version_minor" => "DESC",
            "version_patch" => "DESC",
            "version_build" => "DESC"
        ]
    ]);
    return Response::JSON($resp, 200);
});

Route::Get('/api/servers-status', function() use ($db, $user) {
    $json = ServerInstances::GetStatus();

    if($json === FALSE) {
        return Response::JSON(["error" => "Could not reach netcode-shell"], 500);
    }

    return Response::RawJSON($json, 200);
});

Route::Post('/api/register', function() use ($db, $user) {
    if($user != null) {
        Response::JSON(["error" => "You are already logged in"], 403);
        return;
    }

    JsonPost::Read();
    
    if(!JsonPost::Has('username')) {
        Response::JSON(["error" => "Username was not specified"], 403);
        return;
    }

    if(!JsonPost::Has('password')) {
        Response::JSON(["error" => "Password was not specified"], 403);
        return;
    }

    if(!JsonPost::Has('password_again')) {
        Response::JSON(["error" => "Password was not specified"], 403);
        return;
    }

    $username = JsonPost::Get('username');
    $password = JsonPost::Get('password');
    $passwordAgain = JsonPost::Get('password_again');

    if(strlen($username) > 16 || strlen($username) < 3) {
        Response::JSON(["error" => "Username must be between 3-16 characters long"], 403);
        return;
    }

    if(strlen($password) < 6) {
        Response::JSON(["error" => "I mean, I know, just please do at least 6 characters :D"], 403);
        return;
    }

    if(strlen($password) > 16) {
        Response::JSON(["error" => "Ha! Testing me, I knew it. Do less than 16 characters :P"], 403);
        return;
    }

    if($password != $passwordAgain) {
        Response::JSON(["error" => "Passwords does not match"], 403);
        return;
    }

    if(preg_match("/[a-zA-Z].*/", $username) == 0) {
        Response::JSON(["error" => "YoUr NaMe ShOuLd StArT WiTh A ChArAcTeR, NoT wItH a NuMbEr"], 403);
        return;
    }

    if(preg_match("/[a-zA-Z][a-zA-Z0-9]+/", $username) == 0) {
        Response::JSON(["error" => "Do not try to SQL inject me please, just go with a regular name with only english alphabet and numbers"], 403);
        return;
    }

    if(preg_match("/[a-zA-Z0-9]+/", $password) == 0) {
        Response::JSON(["error" => "Passwords should contain only characters from the english alphabet or numbers"], 403);
        return;
    }

    $query = $db->select("users", ["id"], ["name" => $username]);

    if(count($query) != 0) {
        Response::JSON(["error" => "Username is already taken, text us if you wanna claim yours"], 403);
        return;
    }

    $saltedPw = Crypt::CreateSaltedPassword($password);

    $db->insert("users", ["name" => $username, "password" => $saltedPw]);

    Response::EmptyJSON(200);
});

Route::Post('/api/login', function() use ($db, $user) {
    if($user != null) {
        Response::JSON(["error" => "You are already logged in"], 403);
        return;
    }

    JsonPost::Read();

    if(!JsonPost::Has('username')) {
        Response::JSON(["error" => "Username was not specified"], 403);
        return;
    }

    if(!JsonPost::Has('password')) {
        Response::JSON(["error" => "Password was not specified"], 403);
        return;
    }

    $username = JsonPost::Get('username');
    $rawPassword = JsonPost::Get('password');

    $result = $db->get('users', ["id", "name", "password", "is_banned"], ["name" => $username]);



    $id; $name; $password; $isBanned;
    if($result != null) {
        $id = $result['id'];
        $name = $result['name'];
        $password = $result['password'];
        $isBanned = $result['is_banned'];
    } else {
        Response::JSON(["error" => "Invalid login details"], 403);
        return;
    }

    if(Crypt::VerifyPassword($rawPassword, $password)) {
        $token = null;

        $existingSession = $db->select("sessions", ["id", "hash"], ["user_id" => $id, "expires_at[>]" => \Medoo\Medoo::raw("NOW(6)")]);

        if(count($existingSession) >= 1) {
            $token = $existingSession[0]["hash"];
            $sessionId = $existingSession[0]["id"];

            $db->update("sessions", ["expires_at" => \Medoo\Medoo::raw("DATE_ADD(NOW(6), INTERVAL 14 DAY)")], ["id" => $sessionId]);
        } else {
            $token = Crypt::CreateRandomToken();

            $db->insert("sessions", ["user_id" => $id, "hash" => $token, "expires_at" => \Medoo\Medoo::raw("DATE_ADD(NOW(6), INTERVAL 14 DAY)")]);
        }

        Cookie::Set('netcode-auth', $token, 1209600);

        Response::JSON(["id" => $id, "name" => $username, "is_banned" => $isBanned], 200);
    } else {
        Response::JSON(["error" => "Invalid login details"], 403);
    }
});

Route::Get('/api/logout', function() use ($db, $user) {
    if($user == null) {
        Response::JSON(["error" => "You are not logged in"], 401);
        return;
    }
    $db->update('users', ['session' => null, 'expires_at' => 0], ["id" => $user->id]);
    Cookie::Set('netcode-auth', '', -2000000);
});

Route::Post('/api/create-session', function() use ($db, $user) {
    if($user == null) {
        Response::JSON(["error" => "You are not logged in"], 401);
        return;
    }

    JsonPost::Read();

    if(!JsonPost::Has('max_players')) {
        Response::JSON(["error" => "You must specify the maximum number of players"], 403);
        return;
    }

    $maxPlayers = intval(JsonPost::Get('max_players'));
    $port = intval(JsonPost::Get('port'));
    $tickRate = intval(JsonPost::Get('tick_rate'));

    if($maxPlayers < 2 || $maxPlayers > 16) {
        Response::JSON(["error" => "Max players must be between 2-16"], 403);
        return;
    }

    if($tickRate < 1 || $tickRate > 240) {
        Response::JSON(["error" => "Tick rate must be between 1-240"], 403);
        return;
    }

    $json = ServerInstances::CreateServer($maxPlayers, $port, $user->id, $tickRate);

    if($json === FALSE) {
        return Response::JSON(["error" => "Could not reach netcode-shell"], 500);
    }

    $arr = json_decode($json, true);

    if(!array_key_exists("Port", $arr)) {
        return Response::JSON(["error" => "Unexpected error"], 500);
    }

    Response::RawJSON($json, 200);
});

Route::Get('/api/list-sessions', function() use ($db) {
   $r = $db->select("game_servers",
    [
        "[>]game_sessions" => ["id" => "game_server_id"],
        "[><]users" => ["owner_id" => "id"],
        "[><]versions" => ["version_id" => "id"]
    ],
    [
        "game_servers.id [Int]",
        "game_servers.hostname [String]",
        "game_servers.server_ip [String]",
        "game_servers.game_port [Int]",
        "game_servers.max_players [Int]",
        "users.name(owner) [String]",
        "version" => [
            "versions.version_major(major) [Int]",
            "versions.version_minor(minor) [Int]",
            "versions.version_patch(patch) [Int]",
            "versions.version_build(build) [Int]",
            "hash_sha256" => \Medoo\Medoo::raw("HEX(<versions.hash_sha256>)")
        ],
        "active_players [Int]" => \Medoo\Medoo::raw('SUM(IF(<game_sessions.left_at> IS NULL AND <game_sessions.user_id> IS NOT NULL, 1, 0))')
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
    return Response::Json($r, 200);
});

Route::Run();

?>
