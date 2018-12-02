/* global _ */

/*
 * Complex scripted dashboard
 * This script generates a dashboard object that Grafana can load. It also takes a number of user
 * supplied URL parameters (in the ARGS variable)
 *
 * Return a dashboard object, or a function
 *
 * For async scripts, return a function, this function must take a single callback function as argument,
 * call this callback function with the dashboard object (look at scripted_async.js for an example)
 */

'use strict';

// accessible variables in this scope
var window, document, ARGS, $, jQuery, moment, kbn;

// Setup some variables
var dashboard;

// All url parameters are available via the ARGS object
var ARGS;

// Initialize a skeleton with nothing but a rows array and service object
dashboard = {
    rows : [],
};


dashboard.templating = {
    list: [
        {
            name: 'ApplicationPlatform',
            query: 'SELECT name FROM view;',
            refresh: 1,
            type: 'query',
            datasource: "realopinsight",
        }
    ]
};
// Set a title
dashboard.title = 'SLO/SLA Analysis';

// Set default time
// time can be overridden in the url using from/to parameters, but this is
// handled automatically in grafana core during dashboard initialization
dashboard.time = {
    from: "now-6h",
    to: "now"
};

dashboard.rows.push({
    title: 'Chart',
    height: '300px',
    panels: [
        {
            type: 'graph',
            datasource: 'realopinsight',
            title: "Problem Occurrence Trend",
            span: 6,
            fill: 10,
            linewidth: 0,
            stack: true,
            percentage: true,
            aliasColors: {
                "critical": "#f56",
                "major": "#dd5600",
                "minor": "#fa4",
                "normal": "#4b7",
                "unknown": "#c0c0c0"
            },           
            targets: [
                {
                    "format": "time_series",
                    "group": [],
                    "metricColumn": "none",
                    "rawQuery": false,
                    "select": [
                        [
                            {
                                "params": [
                                    "critical"
                                ],
                                "type": "column"
                            },
                            {
                                "params": [
                                    "critical"
                                ],
                                "type": "alias"
                            }
                        ],
                        [
                            {
                                "params": [
                                    "major"
                                ],
                                "type": "column"
                            },
                            {
                                "params": [
                                    "major"
                                ],
                                "type": "alias"
                            }
                        ],
                        [
                            {
                                "params": [
                                    "normal"
                                ],
                                "type": "column"
                            },
                            {
                                "params": [
                                    "normal"
                                ],
                                "type": "alias"
                            }
                        ],
                        [
                            {

                                "params": [
                                    "unknown"
                                ],
                                "type": "column"
                            },
                            {
                                "params": [
                                    "unknown"
                                ],
                                "type": "alias"
                            }
                        ],
                        [
                            {
                                "params": [
                                    "minor"
                                ],
                                "type": "column"
                            },
                            {
                                "params": [
                                    "minor"
                                ],
                                "type": "alias"
                            }
                        ]
                    ],
                    "table": "qosdata",
                    "timeColumn": "\"timestamp\"",
                    "timeColumnType": "int8",
                    "where": [
                        {
                            "name": "$__unixEpochFilter",
                            "params": [],
                            "type": "macro"
                        },
                        {
                            "datatype": "text",
                            "name": "",
                            "params": [
                                "view_name",
                                "=",
                                "'[[ApplicationPlatform]]'"
                            ],
                            "type": "expression"
                        }
                    ]
                },
                
            ],
            tooltip: {
                shared: true
            }
        },
        {
          aliasColors: {
              "critical": "#f56",
              "major": "#dd5600",
              "minor": "#fa4",
              "normal": "#4b7",
              "unknown": "#c0c0c0"
          },
          "breakPoint": "50%",
          "cacheTimeout": null,
          "combine": {
            "label": "Others",
            "threshold": 0
          },
          "datasource": "realopinsight",
          "fontSize": "80%",
          "format": "short",
          "gridPos": {
            "h": 9,
            "w": 12,
            "x": 0,
            "y": 0
          },
          "hideTimeOverride": false,
          "id": 2,
          "interval": null,
          "legend": {
            "header": "",
            "percentage": true,
            "percentageDecimals": null,
            "show": true,
            "sortDesc": true,
            "values": true
          },
          "legendType": "Under graph",
          "links": [],
          "maxDataPoints": 3,
          "nullPointMode": "connected",
          "pieType": "pie",
          "strokeWidth": "0",
          "targets": [
            {
              "format": "time_series",
              "group": [
                {
                  "params": [
                    "$__interval",
                    "none"
                  ],
                  "type": "time"
                }
              ],
              "metricColumn": "none",
              "rawQuery": true,
              "rawSql": "SELECT\n  NOW() AS time,\n  SUM(CASE WHEN status=0 THEN 1 ELSE 0 END) AS \"normal\",\n  SUM(CASE WHEN status=1 THEN 1 ELSE 0 END) AS \"minor\",\n  SUM(CASE WHEN status=2 THEN 1 ELSE 0 END) AS \"major\",\n  SUM(CASE WHEN status=3 THEN 1 ELSE 0 END) AS \"critical\",\n  SUM(CASE WHEN status=4 THEN 1 ELSE 0 END) AS \"unknown\"\nFROM qosdata\nWHERE\n  $__unixEpochFilter(\"timestamp\") AND\n  view_name = '[[ApplicationPlatform]]'\nGROUP BY time, status ",
              "refId": "A",
              "select": [
                [
                  {
                    "params": [
                      "status"
                    ],
                    "type": "column"
                  },
                  {
                    "params": [
                      "avg"
                    ],
                    "type": "aggregate"
                  },
                  {
                    "params": [
                      "status"
                    ],
                    "type": "alias"
                  }
                ]
              ],
              "table": "qosdata",
              "timeColumn": "\"timestamp\"",
              "timeColumnType": "int8",
              "where": [
                {
                  "name": "$__unixEpochFilter",
                  "params": [],
                  "type": "macro"
                },
                {
                  "name": "",
                  "params": [
                    "view_name",
                    "=",
                    "'[[ApplicationPlatform]]'"
                  ],
                  "type": "expression"
                }
              ]
            }
          ],
          "title": "Overall Status Trend",
          "type": "grafana-piechart-panel",
          "valueName": "total"
        }
    ]
});


return dashboard;