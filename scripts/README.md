
---+--
   |
   +---- \models
   |         |
   |         | **run_test.sh**  (1)        
   |         + \ipv4 
   |         + \ipv4-daas
   |         |      |
   |        ...     +  **run_<tool1>.sh**   (2)  
   |                +  **run_<tool2>.sh** 
   |                   ....  
   |
   +---- **test_<customized>.sh**  (0) 
   |
   +---- 
   |

### Script level 0: 

    scope: environment settings, list of call to "run_test" and output data manipulation

	using: test_<customized>.sh  <loopback_addr> <samples> <output_folder_name>

	a) sets test environment 
	b) defines the constants [scripts_folder_name], [data_block_size]
 	c) calls one or many level1 scripts to generate data clusters in folder <output_folder_name> adding [scripts_folder_name] as file prefix
	d) merges all generated csv files into a single one in the current directory named like: 

		<output_folder_name>_[scripts_folder_name]_<loopback_addr>_[data_block_size]_<samples>_Daytime.csv 
	
	e) reset test environment

          
### Script level 1: 
    
    scope: executes level 2 scripts files present in a folder using the same parameters

	using: run_test.sh <scripts_folder_name> <loopback_addr> <samples> <data_block_size> <output_folder_name>
	
	a) iterates on folder <scripts_folder_name> and execute each finded script  t_[data_block_size]
	b) FACULTATIVE: merges csv data files into a single 



### Script level 2: 

    scope: executes call to a test tool and parse the output to make csv formatted data file 

	using: run_<tool>.sh  <loopback_addr> <data_block_size> samples> <output_folder-path_file-name-prefix>

	a) executes tool 
 	b) parse command output to make csv file named like: 

		<output_folder-path_file-name-prefix>_toolname_[data_block_size]_<samples>_datetime.csv
  
  
### Ultilities

