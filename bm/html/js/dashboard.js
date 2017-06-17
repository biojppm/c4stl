function c4Log() {
  var s = '';
  for(var i = 0; i < arguments.length; ++i) {
    s += arguments[i];
  }
  console.log(s);
  alert(s);
}


function c4GraphOpts(scopeOptions) {

  if(!scopeOptions) scopeOptions = {};

  scopeOptions = Object.assign(scopeOptions, {
    legend: {display: true },
    showLines: true,
    elements: {
      line: { fill: false }
    },
  });

  return scopeOptions;
}


console.log('fonix0');
(function () {
  'use strict';
  console.log('fonix0');

  var dashboardApp = angular.module("dashboardApp", ["chart.js"])

  dashboardApp.config(function (ChartJsProvider) {
    // Configure all charts
    ChartJsProvider.setOptions({
      colors: ['#97BBCD', '#DCDCDC', '#F7464A', '#46BFBD', '#FDB45C', '#949FB1', '#4D5360']
    });
  });

  console.log("fonix1");

  dashboardApp.controller("LineCtrl", function ($scope) {

    console.log('fonix2');
    $scope.series = ['Series A', 'Series B'];
    $scope.labels = ["January", "February", "March", "April", "May", "June", "July", "August", "September"];
    $scope.data = [
      [65, 59, 80, 81, 56, 55, 40, 40, 40],
      [28, 48, 40, 19, 86, 27, 90, 90, 90]
    ];
    $scope.onClick = function (points, evt) {
      console.log(points, evt);
    };
    //$scope.datasetOverride = [{ yAxisID: 'y-axis-1' }, { yAxisID: 'y-axis-2' }];
    $scope.options = {
      scales: {
        yAxes: [
          {
            id: 'y-axis-1',
            type: 'linear',
            display: true,
            position: 'left'
          },
          /*{
            id: 'y-axis-2',
            type: 'linear',
            display: true,
            position: 'right'
          }*/
        ]
      }
    };
    console.log('fonix3');

    $scope.options = c4GraphOpts($scope.options);
  });

  console.log('fonix4');

  dashboardApp.controller('mainController',
                          ['$scope', '$timeout',
                           function($scope, $timeout){
  $scope.labels = [
    moment().add(-7, 'hour').format('HH:00'),
    moment().add(-6, 'hour').format('HH:00'),
    moment().add(-5, 'hour').format('HH:00'),
    moment().add(-4, 'hour').format('HH:00'),
    moment().add(-3, 'hour').format('HH:00'),
    moment().add(-2, 'hour').format('HH:00'),
    moment().add(-1, 'hour').format('HH:00')
  ];
  $scope.series = ['Series A', 'Series B'];
  $scope.data = [
    [1, 2, 3, 4, 5, 6, 7],
    [7, 6, 5, 4, 3, 2, 1]
  ];
  $scope.options = c4GraphOpts($scope.options);

}]);

})();

console.log('fonix5');
