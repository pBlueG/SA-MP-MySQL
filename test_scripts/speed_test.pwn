
//#define IS_R40
#define BENCH_QUERY
//#define BENCH_RESULT_BY_INDEX
//#define BENCH_RESULT_BY_NAME

#define NUM_RUNS 10
#define ITERATIONS 1000


//-----------------
#if defined BENCH_RESULT_BY_INDEX && defined BENCH_RESULT_BY_NAME
    #error Doesn't make much sense testing by index and by name
#endif

#include <a_samp>

#if defined IS_R40
	#include <a_mysql>
	#define MYSQL_VERSION_STRING "R40"
#else
	#include <a_mysql_R39-2>
	#define MYSQL_VERSION_STRING "R39-2"
	#define DEFAULT_MYSQL_HANDLE 1
#endif

enum E_TIME_POINT
{
	START,
	END
}
new TimeData[NUM_RUNS][E_TIME_POINT];
new AverageTime = 0;
new CurrentRun = 0;

#if defined BENCH_RESULT_BY_INDEX || defined BENCH_RESULT_BY_NAME
static const FieldStruct[3][2][] =
{
	{0, "id"},
	{1, "username"},
	{2, "health"}
};
#endif

main() { }

public OnGameModeInit()
{
	#if defined IS_R40
	mysql_log(NONE);
	#else
	mysql_log(LOG_NONE);
	#endif

	mysql_connect("127.0.0.1", "root", "mysql_test", "1234");
	if (mysql_errno(DEFAULT_MYSQL_HANDLE) != 0)
	    return print("Failed to connect!");
	else
	    printf("Connection established, starting benchmark...");
	    
	SetTimer("Deferred_MainStart", 1000, false); //to avoid that "Number of vehicles" message
	
	return 1;
}

forward Deferred_MainStart();
public Deferred_MainStart()
{
	for(new i=0; i < 5; i++)
	    printf("");
    printf("["MYSQL_VERSION_STRING"] running benchmark(s) [runs: "#NUM_RUNS"] [iterations: "#ITERATIONS"]");
	#if defined BENCH_QUERY
	printf("\tquery exec speed");
	#endif
	#if defined BENCH_RESULT_BY_INDEX
	printf("\tresult fetching by index");
	#endif
	#if defined BENCH_RESULT_BY_NAME
	printf("\tresult fetching by name");
	#endif

	StartRun(2000);
	return 1;
}

StartRun(delay = 1000)
{
	SetTimer("Deferred_StartRun", delay, false);
}
forward Deferred_StartRun();
public Deferred_StartRun()
{
    #if defined BENCH_QUERY
    TimeData[CurrentRun][START] = GetTickCount();
	#endif

    for (new i = 1; i <= ITERATIONS; i++)
	{
	    mysql_tquery(DEFAULT_MYSQL_HANDLE,
			"SELECT * FROM `speedtable`",
			"OnTableFetch", "d", i);
	}
	return 1;
}


forward public OnTableFetch(iteration);
public OnTableFetch(iteration)
{
	#if defined BENCH_RESULT_BY_INDEX || defined BENCH_RESULT_BY_NAME
	if(iteration == 1)
    	TimeData[CurrentRun][START] = GetTickCount();
	
	new dest[32];
	new rows, fields;
	cache_get_data(rows, fields);
	for(new f = 0; f < fields; ++f)
	{
		for (new r = 0; r < rows; ++r)
		{
		    #if defined BENCH_RESULT_BY_INDEX
			cache_get_row(r, FieldStruct[f][0][0], dest);
			#else
			cache_get_field_content(r, FieldStruct[f][1], dest);
			#endif
		}
	}
	#endif

	if (iteration == ITERATIONS)
	{
	    TimeData[CurrentRun][END] = GetTickCount();
	    
	    new time = TimeData[CurrentRun][END] - TimeData[CurrentRun][START];
	    printf("\t[%d]: %d (%.4f/iteration)", CurrentRun+1, time, float(time)/float(ITERATIONS));
	    AverageTime += time;
	    
	    if(++CurrentRun != NUM_RUNS)
	    {
	        StartRun();
	    }
	    else
	    {
	        printf("");
			printf("["MYSQL_VERSION_STRING"] benchmark done");
			AverageTime /= NUM_RUNS;
			printf("--->\t %d (%.4f/iteration)", AverageTime, float(AverageTime)/float(ITERATIONS));
			for(new i=0; i < 5; ++i)
				printf("");
		}
	}
	return 1;
}

public OnGameModeExit()
{
	mysql_close(DEFAULT_MYSQL_HANDLE);
	return 1;
}
