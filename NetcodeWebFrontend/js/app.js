var app = angular.module('NetcodeApp',['ngRoute', 'ngCookies']);

function isObject(obj) {
    var type = typeof obj;
    return type === 'function' || type === 'object' && !!obj;
}

app.service('ApiService', ['$http', function($http) {
    return {
        register: function(data, success, fail) {
            $http.post("api/register", data).then(success, fail);
        },
        login: function(data, success, fail) {
            $http.post("api/login", data).then(success, fail);
        },
        logout: function(success, fail) {
            $http.get("api/logout").then(success, fail);
        },
        status: function(success, fail) {
            $http.get("api/status").then(success, fail);
        },
        latestVersion: function (success, fail) {
            $http.get("api/latest-version").then(success, fail);
        },
        serversStatus: function(success, fail) {
            $http.get("api/servers-status").then(success, fail);
        },
        createServer: function(data, success, fail) {
            $http.post("api/create-session", data).then(success, fail);
        }
    };
}]);

app.service('UserService', [function() {
    var user = null;
    return {
        loginUser: function(u) {
            if(u.id === undefined ||
               u.name === undefined || 
               u.is_banned === undefined) {
               return false;
            }
            user = u;
            return true;
        },
        logout: function() {
            user = null;
        },
        userLoggedIn: function() { return user != null; }
    };
}]);

app.config(['$routeProvider', '$locationProvider', function($routeProvider, $locationProvider) {
    $locationProvider.html5Mode(true);

    $routeProvider
    .when('/', {  templateUrl: 'public/main.html', controller: 'HomeController' })
    .when('/technical', { templateUrl: 'public/technical.html' })
    .when('/tutorial', { templateUrl: 'public/tutorial.html' })
    .when('/download', { templateUrl: 'public/download.html', controller: 'DownloadController' })
    .when('/login', {templateUrl: 'public/login.html'})
    .when('/control-panel', { templateUrl: 'public/controlpanel.html', controller: 'ControlPanelController' });
}]);

app.controller('DownloadController', ['$scope', 'ApiService', function ($scope, api) {
    $scope.version = null;
    api.latestVersion(function (data) {
        $scope.version = data.data;
    }, function (data) {
        console.log("error", data);
    });
}]);

app.controller('MainController', ['$scope', 'UserService','ApiService','$cookies', function($scope, us, api, $cookies) {

    $scope.userLoggedIn = us.userLoggedIn;

    api.status(function(data) {
        us.loginUser(data.data);
    }, function(data) {
        console.log("error", data);
    });

    $scope.logout = function() {
        api.logout(function(data){console.log(data);});
        us.logout();
    }
}]);

app.controller('LoginController', ['$scope', 'ApiService','UserService','$location', function($scope, api, us, $location) {
    $scope.loginUsername = "";
    $scope.loginPassword = "";
    $scope.login = function() {
        api.login({username: $scope.loginUsername, password: $scope.loginPassword}, function(data) {
            if(data.data.error == undefined) {
                if(!isObject(data.data)) {
                    $scope.errorMessage = "Bad response";
                    return;
                }

                if(us.loginUser(data.data)) {
                    $location.url('/');
                    console.log("Hello?");
                } else {
                    $scope.errorMessage = "Failed to log in";
                }
            } else {
                $scope.errorMessage = data.data.error;
            }
        }, function(data) {
            $scope.errorMessage = data.toString();
        });
    }
}]);

app.controller('ControlPanelController', ['$scope', 'ApiService', '$interval', function($scope, api, $interval) {
    $scope.maxPlayers = 4;
    $scope.portHint = 8888;
    $scope.tickRate = 60;
    $scope.activeServers = 0;
    $scope.serverInfo = [];
    $scope.shownItems = {};
    $scope.cpError = "";

    $scope.isShown = (index) => {
        return $scope.shownItems[index] && $scope.shownItems[index] == true;
    };

    $scope.toggleItem = (index) => {
        if($scope.isShown(index)) {
            $scope.shownItems[index] = false;
        } else {
            $scope.shownItems[index] = true;
        }
    };

    var updateStatus = function() {
        api.serversStatus(function(data) {
            if(data.data.error == undefined) {
                $scope.activeServers = data.data.server_count;
                $scope.serverInfo = data.data.server_info;
                $scope.cpError = "";
            } else {
                $scope.activeServers = 0;
                $scope.serverInfo = [];
                $scope.cpError = data.data.error;
            }
        }, function(data) {
            console.log("error", data);
        });
    }
    $interval(function() {
        updateStatus();
    }, 5000);

    $scope.createServer = function() {
        var intervalInMs = Math.max(1, Math.floor(1000 / $scope.tickRate));

        api.createServer({max_players: $scope.maxPlayers, port: $scope.portHint, interval: intervalInMs}, function(data) {
            updateStatus();
        }, function(data) {
            console.log("error", data);
        });
    }
}]);

app.controller('HomeController', ['$scope', 'ApiService','$timeout','$interval', function($scope, api, $timeout, $interval) {
    $scope.successMessage = "";
    $scope.errorMessage = "";

    var hide = function() {
        $scope.successMessage = "";
        $scope.errorMessage = "";
    };

    hide();

    var delayedHide = function() {
        $timeout(hide, 10000);
    }

    $scope.register = () => {
        hide();
        api.register({
            "username": $scope.username,
            "password": $scope.password,
            "password_again": $scope.password_again
        }, function(data) {
            if(data.data.error != undefined) {
                $scope.errorMessage = data.data.error;
            } else {
                $scope.username = "";
                $scope.password = "";
                $scope.password_again = "";
                $scope.successMessage = "You may now log in";
            }
            delayedHide();
        }, function(data) {
            $scope.errorMessage = "Something went wrong, please contact us";
            delayedHide();
        });
    }
}]);
