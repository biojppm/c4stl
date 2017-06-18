function c4Log() {
  var s = '';
  for(var i = 0; i < arguments.length; ++i) {
    s += arguments[i];
  }
  console.log(s);
  alert(s);
}

// http://planetozh.com/blog/2008/04/javascript-basename-and-dirname/
function c4Basename(path) {
  return path.replace(/\\/g,'/').replace( /.*\//, '' );
}
function c4Dirname(path) {
  return path.replace(/\\/g,'/').replace(/\/[^\/]*$/, '');;
}

// https://stackoverflow.com/questions/280634/endswith-in-javascript
/*String.prototype.endsWith = function(suffix) {
    return this.indexOf(suffix, this.length - suffix.length) !== -1;
};*/

// https://stackoverflow.com/questions/14446447/javascript-read-local-text-file
function c4ReadTextFile(file) {
  var rawFile = new XMLHttpRequest();
  var txt = null;
  rawFile.open("GET", file, false);
  rawFile.onreadystatechange = function () {
    if(rawFile.readyState === 4)
    {
      if(rawFile.status === 200 || rawFile.status == 0)
      {
        txt = rawFile.responseText;
        console.log("XHR: received:\n" + txt);
      }
    }
  }
  rawFile.send(null);
  return txt;
}

// Return array of string values, or NULL if CSV string not well formed.
// https://stackoverflow.com/questions/8493195/how-can-i-parse-a-csv-string-with-javascript-which-contains-comma-in-data
function c4CSVtoArray(text) {
  var re_valid = /^\s*(?:'[^'\\]*(?:\\[\S\s][^'\\]*)*'|"[^"\\]*(?:\\[\S\s][^"\\]*)*"|[^,'"\s\\]*(?:\s+[^,'"\s\\]+)*)\s*(?:,\s*(?:'[^'\\]*(?:\\[\S\s][^'\\]*)*'|"[^"\\]*(?:\\[\S\s][^"\\]*)*"|[^,'"\s\\]*(?:\s+[^,'"\s\\]+)*)\s*)*$/;
  var re_value = /(?!\s*$)\s*(?:'([^'\\]*(?:\\[\S\s][^'\\]*)*)'|"([^"\\]*(?:\\[\S\s][^"\\]*)*)"|([^,'"\s\\]*(?:\s+[^,'"\s\\]+)*))\s*(?:,|$)/g;
  // Return NULL if input string is not well formed CSV string.
  if (!re_valid.test(text)) return null;
  var a = [];                     // Initialize array to receive values.
  text.replace(
    re_value, // "Walk" the string using replace with callback.
    function(m0, m1, m2, m3) {
      // Remove backslash from \' in single quoted values.
      if      (m1 !== undefined) a.push(m1.replace(/\\'/g, "'"));
      // Remove backslash from \" in double quoted values.
      else if (m2 !== undefined) a.push(m2.replace(/\\"/g, '"'));
      else if (m3 !== undefined) a.push(m3);
      return ''; // Return empty string.
    }
  );
  // Handle special case of empty last value.
  if (/,\s*$/.test(text)) a.push('');
  return a;
};

// https://stackoverflow.com/questions/28543821/convert-csv-lines-into-javascript-objects
function c4BenchmarkTextToObj(txt) {
  var obj = [];
  var arr = txt.split('\n');
  var headers = null;
  for(var i = 0; i < arr.length; i++) {
    if(!arr[i]) continue;
    if(!headers) {
      if(arr[i].startsWith('name')) {
        headers = arr[i].split(',');
      }
      continue;
    }
    var data = c4CSVtoArray(arr[i]);
    var res = {};
    for(var j = 0; j < data.length; j++) {
      res[headers[j].trim()] = data[j].trim();
    }
    obj.push(res);
  }
  console.log("bmobj:");
  console.log(obj);
  return obj;
}

function c4ReadBenchmarkResults(fileName) {
  var path = window.location.href;
  path = c4Dirname(path);
  path += '/' + fileName;
  var txt = c4ReadTextFile(path);
  var obj = c4BenchmarkTextToObj(txt);
  // we assume the benchmark is for a sequence of n,
  // with two additional entries for complexity and RMS at end.
  var bm = {};
  bm.n = [];
  // now create arrays in the bm object
  // select the first benchmark; it should have all data
  var bm0 = obj[0];
  for(var p in bm0) {
    if (bm0.hasOwnProperty(p) && p != 'name') {
      bm[p] = [];
    }
  }
  console.log("CARALHO:\n");
  console.log(bm);
  var currname = null;
  for(var i = 0; i < obj.length; i++) {
    var r = obj[i];
    var name = r.name.replace(/"/g, '');
    console.log(r.name + ' -----> ' + name);
    if(name.endsWith('_BigO')) {
      bm.complexity_label = obj.bytes_per_second;
      bm.complexity_real_time = obj.real_time;
      bm.complexity_cpu_time = obj.cpu_time;
    } else if(name.endsWith('_RMS')) {
      bm.complexity_rms_real_time = obj.real_time;
      bm.complexity_rms_cpu_time = obj.cpu_time;
    } else {
      var spl = name.split('/');
      var n = spl.pop();
      bm.n.push(n);
      name = spl.join('/');
      if(!currname) {
        currname = name;
      } else {
        console.assert(name == currname, name, currname);
      }
      bm.name = name;
      for(var p in bm0) {
        if(bm0.hasOwnProperty(p) && p != 'name') {
          bm[p].push(r[p])
        }
      }
    }
  }
  console.log("CONA:\n");
  console.log(bm);
  return bm;
}

function c4XYData(x, y) {
  var ret = []
  for(var i = 0; i < x.length; ++i) {
    ret.push({x: x[i], y: y[i]});
  }
  return ret;
}

console.log('fonix0');
(function () {
  'use strict';
  console.log('fonix0');

  var dashboardApp = angular.module("dashboardApp", ["chart.js"])

  // Configure all charts
  // http://www.chartjs.org/docs/latest/
  dashboardApp.config(function (ChartJsProvider) {
    ChartJsProvider.setOptions({
      colors: ['#97BBCD', '#DCDCDC', '#F7464A', '#46BFBD', '#FDB45C', '#949FB1', '#4D5360'],
      legend: {display: true },
      showLines: true,
      elements: {
        line: { fill: false },
        xy: { fill: false },
      },
      scales: {
        xAxes: [
          {
            id: 'x-axis-1',
            type: 'logarithmic',
            display: true,
            position: 'bottom'
          },
        ],
        yAxes: [
          {
            id: 'y-axis-1',
            type: 'logarithmic',
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
    });
  });

  console.log("fonix1");

  var stdlist = c4ReadBenchmarkResults('res.csv');
  var flatlist = c4ReadBenchmarkResults('res-flr.csv');
  console.log("fonix1.1");

  dashboardApp.controller("CPUTimeCtrl", function ($scope) {
    console.log('fonix2');
    $scope.series = [stdlist.name, flatlist.name];
    $scope.data = [
      c4XYData(stdlist.n, stdlist.cpu_time),
      c4XYData(flatlist.n, flatlist.cpu_time),
    ];
    /*$scope.onClick = function (points, evt) {
      console.log(points, evt);
    };*/
    console.log('fonix3');
  });

  dashboardApp.controller("ItemsPerSecCtrl", function ($scope) {
    console.log('fonix2');
    $scope.series = [stdlist.name, flatlist.name];
    $scope.data = [
      c4XYData(stdlist.n, stdlist.items_per_second),
      c4XYData(flatlist.n, flatlist.items_per_second),
    ];
    /*$scope.onClick = function (points, evt) {
      console.log(points, evt);
    };*/
    console.log('fonix3');
  });

  dashboardApp.controller("BytesPerSecCtrl", function ($scope) {
    console.log('fonix2');
    $scope.series = [stdlist.name, flatlist.name];
    $scope.data = [
      c4XYData(stdlist.n, stdlist.bytes_per_second),
      c4XYData(flatlist.n, flatlist.bytes_per_second),
    ];
    /*$scope.onClick = function (points, evt) {
      console.log(points, evt);
    };*/
    console.log('fonix3');
  });

  console.log('fonix4');

})();

console.log('fonix5');
