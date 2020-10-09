<?php

require_once 'Response.php';

class Route {

    private $method;
    private $url;
    private $callback;

    private static $routes = [];

    private function __construct($m, $u, $c) {
        $this->method = $m;
        $this->url = $u;
        $this->callback = $c;
    }

    private static function Insert($method, $url, $callback) {
        if(!array_key_exists($method, self::$routes)) {
            self::$routes[$method] = [];
        } 
        self::$routes[$method][] = new Route($method, $url, $callback);
    }

    public static function Get($url, $callback) {
        self::Insert("GET", $url, $callback);
    }

    public static function Post($url, $callback) {
        self::Insert("POST", $url, $callback);
    }

    public static function Run() {
        $method = $_SERVER['REQUEST_METHOD'];
        $url = $_SERVER['REQUEST_URI'];
        
        if(array_key_exists($method, self::$routes)) {
            foreach (self::$routes[$method] as $route) {
                // do matching, for now exact match:
                if($route->url == $url) {
                    ($route->callback)();
                    return;
                }
            }
        }
        
        Response::JSON(["error" => 404], 404);
    }

}
