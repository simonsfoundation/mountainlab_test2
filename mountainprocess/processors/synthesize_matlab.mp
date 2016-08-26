{
	"processors":[
		{
			"name":"synthesize_timeseries_001_matlab",
			"version":"0.1",
			"description":"",
			"inputs":[],
			"outputs":[
				{"name":"waveforms"},
				{"name":"timeseries"},
				{"name":"firings_true"}
			],
			"parameters":[],
			"exe_command":"$(basepath)/run_matlab.sh $(basepath)/synthesize_matlab synthesize_timeseries_001('$timeseries','$firings_true','$waveforms');"
		}
	]
}
