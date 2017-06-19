function c4Log() {
  var s = '';
  for(var i = 0; i < arguments.length; ++i) {
    s += arguments[i];
  }
  console.log(s);
  alert(s);
}

// https://stackoverflow.com/questions/728360/how-do-i-correctly-clone-a-javascript-object
function c4Clone(obj) {
  var copy;

  // Handle the 3 simple types, and null or undefined
  if (null == obj || "object" != typeof obj) return obj;

  // Handle Date
  if (obj instanceof Date) {
    copy = new Date();
    copy.setTime(obj.getTime());
    return copy;
  }

  // Handle Array
  if (obj instanceof Array) {
    copy = [];
    for (var i = 0, len = obj.length; i < len; i++) {
      copy[i] = c4Clone(obj[i]);
    }
    return copy;
  }

  // Handle Object
  if (obj instanceof Object) {
    copy = {};
    for (var attr in obj) {
      if (obj.hasOwnProperty(attr)) copy[attr] = c4Clone(obj[attr]);
    }
    return copy;
  }

  throw new Error("Unable to copy obj! Its type isn't supported.");
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
        //console.log("XHR: received:\n" + txt);
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
        headers = c4CSVtoArray(arr[i])
        obj.csv_names = headers
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
  var csv = c4BenchmarkTextToObj(txt);
  // we assume the benchmark is for a sequence of n,
  // with two additional entries for complexity and RMS at end.
  var bm = {};
  bm.csv = csv;
  bm.n = [];
  // now create arrays in the bm object
  // select the first benchmark; it should have all data
  var bm0 = csv[0];
  for(var p in bm0) {
    if (bm0.hasOwnProperty(p) && p != 'name') {
      bm[p] = [];
    }
  }
  var currname = null;
  for(var i = 0; i < csv.length; i++) {
    var r = csv[i];
    var name = r.name.replace(/"/g, '');
    if(name.endsWith('_BigO')) {
      bm.complexity_label = csv.bytes_per_second;
      bm.complexity_real_time = csv.real_time;
      bm.complexity_cpu_time = csv.cpu_time;
    } else if(name.endsWith('_RMS')) {
      bm.complexity_rms_real_time = csv.real_time;
      bm.complexity_rms_cpu_time = csv.cpu_time;
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
  console.log("Loaded benchmark results for " + fileName + ":\n");
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


function c4LoadChart(thiscase, params={}) {
  console.log('fonix0');

  var speedup_baseline = null
  for(var i = 0; i < thiscase.entries.length; ++i) {
    var e = thiscase.entries[i];
    if(e.baseline) {
      speedup_baseline = e
      thiscase.baseline = speedup_baseline
      break
    }
  }

  var r = {
    series: [],
    data: [],
  };
  for(var i = 0; i < thiscase.entries.length; ++i) {
    var e = thiscase.entries[i];
    if(!e.data) {
      e.data = c4ReadBenchmarkResults(e.file);
    }
    var xy = c4XYData(e.data[params.x.field], e.data[params.y.field]);
    r.series.push(e.name);
    r.data.push(xy);
  }
  var app = angular.module("dashboardApp") // get existing
  var options = c4Clone(app.c4DefaultChartOptions)

  console.log('fonix1');
  app.controller(params.idCtrl, function ($scope) {
    console.log('fonix2');
    $scope.series = r.series;
    $scope.data = r.data;
    /*$scope.onClick = function (points, evt) {
      console.log(points, evt);
    };*/
    $scope.options = options
    $scope.options.scales.xAxes[0].scaleLabel = {
      display: true,
      labelString: params.x.label,
    }
    $scope.options.scales.yAxes[0].scaleLabel = {
      display: true,
      labelString: params.y.label,
    }
    console.log('fonix3');
  });
  console.log('fonix4');

  tableColumns = [{field: params.x.field, title: params.x.field},]
  tableColumnsSpeedup = [{field: params.x.field, title: params.x.field},]
  for(var i = 0; i < thiscase.entries.length; ++i) {
    var e = thiscase.entries[i];
    tableColumns.push({
      field: e.name,
      title: e.name,
    });
    tableColumnsSpeedup.push({
      field: e.name + '[speedup]',
      title: e.name + '[speedup]',
    });
  }
  tableData = []
  tableDataSpeedup = []
  for(var j = 0; j < e.data[params.x.field].length; ++j) {
    var line = {}
    var lineSpeedup = {}
    for(var i = 0; i < thiscase.entries.length; ++i) {
      var e = thiscase.entries[i];
      var n = null;
      if(!n) {
        n = e.data[params.x.field][j];
        line[params.x.field] = n;
        lineSpeedup[params.x.field] = n;
      } else {
        assert(n == e.data[params.x.field][j], n, e.data[params.x.field][j])
      }
      line[e.name] = e.data[params.y.field][j];
      var speedup = speedup_baseline.data[params.y.field][j] / e.data[params.y.field][j]
      if(params.reciprocal) {
        speedup = 1. / speedup
      }
      lineSpeedup[e.name + '[speedup]'] = speedup
    }
    tableData.push(line);
    tableDataSpeedup.push(lineSpeedup);
  }
  $('#' + params.idCtrl + '_table').bootstrapTable({
    columns: tableColumns,
    data: tableData,
  })
  $('#' + params.idCtrl + '_table_speedup').bootstrapTable({
    columns: tableColumnsSpeedup,
    data: tableDataSpeedup,
  })
  return r;
}


function c4LoadCase(thiscase) {
  thiscase = {
    name:'list / push_back() with reserve / int',
    entries:[
      {name:'std::list<int>', file:'res.csv', baseline:true},
      {name:'c4::flat_list__buf<int>', file:'res-flr.csv'},
      {name:'c4::flat_list__paged<int>', file:'res-flrp.csv'},
      {name:'c4::flat_list__paged_rt<int>', file:'res-flrp_rt.csv'},
    ],
  };

  // var ph1 = angular.element(document.querySelector('#page-header'))
  $('#page-header').html(thiscase.name)

  var app = angular.module("dashboardApp") // get existing

  c4LoadChart(thiscase, {
    idCtrl: "CPUTimeCtrl",
    x: {field:"n", label:"n"},
    y: {field:"cpu_time", label:"ns"}, // FIXME: the label must be read from the benchmark data
  })
  c4LoadChart(thiscase, {
    idCtrl: "ItemsPerSecCtrl",
    reciprocal: true,
    x: {field:"n", label:"n"},
    y: {field:"items_per_second", label:"items/s"},
  })
  c4LoadChart(thiscase, {
    idCtrl: "BytesPerSecCtrl",
    reciprocal: true,
    x: {field:"n", label:"n"},
    y: {field:"bytes_per_second", label:"bytes/s"},
  })


  var tableColumns = []
  var csv_names = thiscase.entries[0].data.csv.csv_names
  for(var k = 0; k < csv_names.length; ++k) {
    var n = csv_names[k]
    tableColumns.push({field: n, title: n})
  }
  var tableData = []
  for(var i = 0; i < thiscase.entries.length; ++i) {
    var e = thiscase.entries[i].data.csv
    for(var j = 0; j < e.length; ++j) {
      var line = {}
      for(var k = 0; k < csv_names.length; ++k) {
        var n = csv_names[k]
        line[n] = e[j][n]
      }
      tableData.push(line)
    }
    tableData.push({})
  }
  $('#fullResultsTable').bootstrapTable({
    columns: tableColumns,
    data: tableData,
  })
}


(function () {
  'use strict';

  console.log('c4: creating app...');
  var dashboardApp = angular.module("dashboardApp", ["chart.js"]) // create
  console.log('c4: app created.');

  // save this object for reusing later
  dashboardApp.c4DefaultChartOptions = {
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
  }

  // Configure all charts
  // http://www.chartjs.org/docs/latest/
  dashboardApp.config(function (ChartJsProvider) {
    ChartJsProvider.setOptions(dashboardApp.c4DefaultChartOptions);
  });

  c4LoadCase({});

})();
