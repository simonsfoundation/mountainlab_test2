{
	"processors":[
		{
			"name":"copy2",
			"version":"0.1",
			"description":"",
			"inputs":[{"name":"input"}],
			"outputs":[{"name":"output"}],
			"parameters":[],
			"exe_command":"cp $input$ $output$"
		},
		{
			"name":"copy_with_delay",
			"version":"0.1",
			"description":"",
			"inputs":[{"name":"input"}],
			"outputs":[{"name":"output"}],
			"parameters":[],
			"exe_command":"$(basepath)/copy_with_delay.sh $input$ $output$"
		}
	]
}
