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
      if (obj.hasOwnProperty(attr)) {
        copy[attr] = c4Clone(obj[attr]);
      }
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
      bm.complexity_label = r.time_unit;
      bm.complexity_real_time = r.real_time;
      bm.complexity_cpu_time = r.cpu_time;
    } else if(name.endsWith('_RMS')) {
      bm.complexity_rms_real_time = r.real_time;
      bm.complexity_rms_cpu_time = r.cpu_time;
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


function c4LoadChartAndTable(thiscase, params={}) {

  // process the data for the chart
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

  // create the chart
  app = angular.module("dashboardApp");
  app.controller(params.y.field + '_ctrl', function ($scope) {
    var options = c4Clone(app.c4DefaultChartOptions)
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
  });

  c4LoadDataToChartTables(thiscase, params)
}


function c4LoadDataToComplexityTable(thiscase) {
  // process the data for the table
  tableColumns = [
    {field: 'name', title: 'name'},
    {field: 'complexity', title: 'complexity'},
    {field: 'value', title: 'factor'},
    {field: 'rms', title: 'rms'},
    {field: 'speedup', title: 'factor speedup'},
  ]
  tableData = []
  for(var i = 0; i < thiscase.entries.length; ++i) {
    var e = thiscase.entries[i];
    line = {
      name: e.name,
      complexity: e.data.complexity_label,
      value: e.data.complexity_cpu_time,
      rms: e.data.complexity_rms_cpu_time,
      speedup: thiscase.baseline.data.complexity_cpu_time / e.data.complexity_cpu_time
    }
    tableData.push(line);
  }

  $('#complexity_table').bootstrapTable({
    columns: tableColumns,
    data: tableData,
  })
}


function c4LoadDataToFullResultsTable(thiscase) {
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


function c4LoadDataToChartTables(thiscase, params) {
  // process the data for the table
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
      var speedup = thiscase.baseline.data[params.y.field][j] / e.data[params.y.field][j]
      if(params.reciprocal) {
        speedup = 1. / speedup
      }
      lineSpeedup[e.name + '[speedup]'] = speedup
    }
    tableData.push(line);
    tableDataSpeedup.push(lineSpeedup);
  }

  // set the table(s)
  $('#' + params.y.field + '_table').bootstrapTable({
    columns: tableColumns,
    data: tableData,
  })
  $('#' + params.y.field + '_table_speedup').bootstrapTable({
    columns: tableColumnsSpeedup,
    data: tableDataSpeedup,
  })
}

function c4LoadCase(thiscase) {

  for(var i = 0; i < thiscase.entries.length; ++i) {
    var e = thiscase.entries[i];
    if(e.baseline) {
      thiscase.baseline = e
      break
    }
  }
  if(!thiscase.baseline) {
    thiscase.baseline = thiscase.entries[0]
  }

  // set the title and description
  $('#page-header').html(thiscase.name)

  $('#benchmark-desc').html(thiscase.desc)

  // load the sections
  c4LoadChartAndTable(thiscase, {
    x: {field:"n", label:"n"},
    y: {field:"cpu_time", label:"cpu_time (ns)"}, // FIXME: the label must be read from the benchmark data
  })
  c4LoadChartAndTable(thiscase, {
    reciprocal: true,
    x: {field:"n", label:"n"},
    y: {field:"items_per_second", label:"items/s"},
  })
  c4LoadChartAndTable(thiscase, {
    reciprocal: true,
    x: {field:"n", label:"n"},
    y: {field:"bytes_per_second", label:"bytes/s"},
  })


  c4LoadDataToComplexityTable(thiscase)

  c4LoadDataToFullResultsTable(thiscase)
}

// this is here as a placeholder; ultimately the cases will be loaded from a
// json file written by the benchmark creator (probably cmake calling python)
var c4Cases = {
  stream: [
  ],
  span: [
  ],
  vector: [
  ],
  map: [{
    name:'map / insert() with reserve / int',
    desc:'bla bla bla map benchmark',
    entries:[
      {name:'std::map<int>', file:'res.csv', baseline:true},
      {name:'c4::flat_map<int,size_t,ptr>', file:'res-flr.csv'},
      {name:'c4::flat_map<int,size_t,paged>', file:'res-flrp.csv'},
      {name:'c4::flat_map<int,size_t,paged_rt>', file:'res-flrp_rt.csv'},
    ],
  }],
  list: [
    {
      name:'list / push_back() / int',
      desc:'bla bla bla',
      entries:[
        {name:'std::list<int>', file:'res.csv', baseline:true},
        {name:'c4::flat_list<int,size_t,ptr>', file:'res-flr.csv'},
        {name:'c4::flat_list<int,size_t,paged>', file:'res-flrp.csv'},
        {name:'c4::flat_list<int,size_t,paged_rt>', file:'res-flrp_rt.csv'},
      ],
    },
    {
      name:'list / push_back() with reserve / int',
      desc:'benchmarks time taken to push n elements to an initially empty list. The list capacity is reserved before pushing the data. Complexity measures total time for the N elements, not time per push_back().',
      entries:[
        {name:'std::list<int>', file:'res.csv', baseline:true},
        {name:'c4::flat_list<int,size_t,ptr>', file:'res-flr.csv'},
        {name:'c4::flat_list<int,size_t,paged>', file:'res-flrp.csv'},
        {name:'c4::flat_list<int,size_t,paged_rt>', file:'res-flrp_rt.csv'},
      ],
    }
  ],
};
var c4CurrentCase = {
  topic: null,
  bm: null
}

function c4LoadBenchmarkNav() {
  var nav = ""
  for (var topic in c4Cases) {
    if (c4Cases.hasOwnProperty(topic)) {
      nav += '<li id="bm-nav-topic-' + topic + '"><a href="#" ';
      nav += 'onclick="c4SelectTopic(\''+topic+'\')">';
      nav += '<pre>' + topic + '</pre></a></li>'
    }
  }
  $('#bm-nav').html(nav);
  c4SelectTopic(null);
}


function c4SelectTopic(which) {
  if(which != null && which == c4CurrentCase.topic) return;
  console.log("c4SelectTopic:", which)
  if(c4CurrentCase.topic) {
    $('#bm-nav-topic-' + c4CurrentCase.topic).removeClass("active");
  }
  c4CurrentCase.topic = which;
  if(!which) {
    $('#benchmark-topic-div').addClass("c4-hidden");
    $('#jumbo-tip').html('Start by selecting a topic from the menu above.');
  } else {
    $('#bm-nav-topic-' + which).addClass("active");
    $('#benchmark-topic-div').removeClass("c4-hidden");
    $('#benchmark-topic-header').html(which + ' - available benchmarks.');
    $('#jumbo-tip').html('Currently selected topic: ' + which);

    tableCols = [
      {field: 'id', title: 'id'},
      {field: 'name', title: 'name'},
      {field: 'desc', title: 'Description'},
    ]
    tableData = []
    console.log(c4Cases[which]);
    for(var i = 0; i < c4Cases[which].length; ++i) {
      var c = c4Cases[which][i]
      var line = {
        id: i,
        name: '<a href="#" onclick="c4SelectBenchmark('+i+');">' + c.name + '</a>',
        desc: c.desc,
      };
      tableData.push(line);
    }
    $('#benchmark-topic-table').bootstrapTable("destroy");
    $('#benchmark-topic-table').bootstrapTable({
      columns: tableCols,
      data: tableData,
    });
  }
  c4SelectBenchmark(null);
  console.log("c4SelectTopic:", which, " -- done")
}


function c4SelectBenchmark(which) {
  if(which != null && which == c4CurrentCase.bm) return;
  console.log("c4SelectBenchmark:", which);
  c4CurrentCase.bm = which;
  if(which) {
    which_case = c4Cases[c4CurrentCase.topic][which];
    c4LoadCase(which_case);
    $('#benchmark-results').removeClass("c4-hidden");
  } else {
    $('#benchmark-results').addClass("c4-hidden");
  }
  console.log("c4SelectBenchmark:", which, " -- done");
}


(function () {
  'use strict';

  console.log('c4: creating app...');
  var dashboardApp = angular.module("dashboardApp", ["chart.js"]) // create
  console.log('c4: app created.');

  // save this object for reusing later
  dashboardApp.c4DefaultChartOptions = {
    // http://www.chartjs.org/docs/latest/
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
  dashboardApp.config(function (ChartJsProvider) {
    ChartJsProvider.setOptions(dashboardApp.c4DefaultChartOptions);
  });

  c4LoadBenchmarkNav();

})();
