/**
@file		test_flat_file.c
@author		Scott Ronald Fazackerley

@details	Tests base operations for open address hash map
 */

#include "test_flat_file.h"

#define STD_KV_SIZE 10
#define TEST_FILE	"file.bin"


/**
@brief			Helper function to visualize hashmap contents

@param 			map
					The hashmap to visualize.
 */
void
check_flat_file(
	ff_file_t 	*flat_file
)
{
	int bucket_size = flat_file->super.record.key_size + flat_file->super.record.value_size + sizeof(char);

	frewind(flat_file->file_ptr);
	f_file_record_t * record;
	record = (f_file_record_t *)malloc(bucket_size);
	DUMP(bucket_size,"%i");
	while (!feof(flat_file->file_ptr))
	{
		int j;
		DUMP((int) fread(record,bucket_size,1,flat_file->file_ptr),"%d");
		printf("reading\n");
		fflush(stdout);
		for (j = 0; j < bucket_size; j++)
		{
			printf("%c ", *((char *)record) + j);
		}
		printf("\n");
		fflush(stdout);
	}
	//and now check key positions
}
void
initialize_flat_file(
	record_info_t	*record,
	ff_file_t 	*flat_file
)
{
	ff_initialize(flat_file, /*dictionary_compare_signed_value,*/ flat_file->super.key_type, record->key_size, record->value_size);
	flat_file->super.compare = dictionary_compare_signed_value;
}

void
initialize_flat_file_std_conditions(
	ff_file_t	*flat_file
)
{

	record_info_t * record = (record_info_t*)malloc(sizeof(record_info_t));
	record->key_size 		= 4;
	record->value_size 		= 10;
	flat_file->super.key_type 	= key_type_numeric_signed;
	initialize_flat_file(record, flat_file);
	free(record);
}

/**
@brief 		Tests a simple insert into map and reads results directly from map

@param 		tc
				plank_unit_test_t
 */
void
test_flat_file_simple_insert(
	planck_unit_test_t	*tc
)
{
	ff_file_t flat_file;			//create handler for flat file
	int i;

	initialize_flat_file_std_conditions(&flat_file);

	//check_file_map(&map);

	// Manually populate records
	record_info_t record 			= flat_file.super.record;

	int bucket_size 				= sizeof(char)
											+ record.key_size + record.value_size;
	//rewind
	frewind(flat_file.file_ptr);


		for (i = 0; i<STD_KV_SIZE; i++)
		{
			//build up the value
			char str[10];
			sprintf(str,"%02i is key",i);
			ff_insert(&flat_file, (ion_key_t)(&i), (unsigned char *)str);			//this is will wrap

		}

		for (i = 0; i<STD_KV_SIZE; i++)
		{
			//set the position in the file
			fseek(flat_file.file_ptr,flat_file.start_of_data+(i*bucket_size),SEEK_SET);

			status_t status;			//= ((hash_bucket_t *)(map.entry + ((((i+offset)%map.map_size)*bucket_size )%(map.map_size*bucket_size))))->status;
			int key;					//= *(int *)(((hash_bucket_t *)(map.entry + ((((i+offset)%map.map_size)*bucket_size )%(map.map_size*bucket_size))))->data );
			unsigned char value[10];	//= (ion_value_t)(((hash_bucket_t *)(map.entry + ((((i+offset)%map.map_size)*bucket_size )%(map.map_size*bucket_size))))->data + sizeof(int));

			fread(&status, SIZEOF(STATUS),1, flat_file.file_ptr);
			fread(&key, flat_file.super.record.key_size, 1, flat_file.file_ptr);
			fread(value, flat_file.super.record.value_size, 1, flat_file.file_ptr);
			//build up expected value
			char str[10];
			sprintf(str,"%02i is key", i);
			PLANCK_UNIT_ASSERT_TRUE(tc, status		== IN_USE);
			PLANCK_UNIT_ASSERT_TRUE(tc, key 		== i);
			PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *)value, (char *)str);
		}
	fclose(flat_file.file_ptr);
	fremove(TEST_FILE);
}
/**
@brief 		Tests a simple insert into dictionary and simple query

@details	Tests a simple insert into dictionary and simple query with the
			write_concern set to insert only
@param 		tc
				plank_unit_test_t
 */
void
test_flat_file_simple_insert_and_query(
	planck_unit_test_t	*tc
)
{
	ff_file_t flat_file;								//create handler for file
	int i;

	initialize_flat_file_std_conditions(&flat_file);

	for (i = 0; i< STD_KV_SIZE; i++)
	{
		//build up the value
		char str[10];
		sprintf(str,"%02i is key",i);
		ff_insert(&flat_file, IONIZE(i, int), (ion_value_t)str);			//this is will wrap
	}

	printf("starting lookup\n");
	for (i = 0; i<STD_KV_SIZE; i++)
	{
		ion_value_t value;
		value = (ion_value_t)malloc(flat_file.super.record.value_size);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok 	== ff_query(&flat_file,(ion_key_t)&i, value));
		//build up expected value
		char str[10];
		sprintf(str,"%02i is key",i);
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *)value, str);
		if (value != NULL)							//must free value after query
		{
			free(value);
		}
	}

	fclose(flat_file.file_ptr);
	fremove(TEST_FILE);

}

void
test_flat_file_initialize(
	planck_unit_test_t	*tc
)
{

	/* this is required for initializing the hash map and should come from the dictionary */
	record_info_t record;
	record.key_size = 4;
	record.value_size = 10;
	ff_file_t flat_file;

	flat_file.super.key_type = key_type_numeric_signed;			//default to use int key type

	initialize_flat_file( &record, &flat_file);

	//valid correct map settings
	PLANCK_UNIT_ASSERT_TRUE(tc, flat_file.super.record.key_size 		== record.key_size);
	PLANCK_UNIT_ASSERT_TRUE(tc, flat_file.super.record.value_size		== record.value_size);
	PLANCK_UNIT_ASSERT_TRUE(tc, flat_file.write_concern 				== wc_insert_unique);

	fclose(flat_file.file_ptr);
	fremove(TEST_FILE);
}

/**
@brief 		Tests a simple delete from dictionary

@details	Tests a simple delete from dictionary.  Builds a test collection
			and then deletes records one by one, checking that each record has
			been successfully deleted and that the rest of the map has not
			been perturbed.

@param 		tc
				plank_unit_test_t
 */
void
test_flat_file_simple_delete(
	planck_unit_test_t	*tc
)
{
	ff_file_t file;								//create handler for hashmap
	int i,j;


	initialize_flat_file_std_conditions(&file);

	for (i = 0; i<STD_KV_SIZE; i++)
	{
		//build up the value
		char str[10];
		sprintf(str,"%02i is key",i);
		ff_insert(&file, (ion_key_t)(&i), (ion_value_t)str);			//this is will wrap
	}

	ion_value_t value;
	value = (ion_value_t)malloc(file.super.record.value_size);

	for (j = 0; j<STD_KV_SIZE;j++)
	{

		//delete the record (single record)
		ion_status_t status;
		status = ff_delete(&file, (ion_key_t)(&j));
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok				== status.err);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1					== status.count);
		//check to make sure that the record has been deleted
		PLANCK_UNIT_ASSERT_TRUE(tc, err_item_not_found	== ff_query(&file,(ion_key_t)(&j), value));

		//and then check to make sure that the rest of the map is undisturbed
		for (i = j+1; i<STD_KV_SIZE; i++)
		{
			ion_value_t value2;
			value2 = (ion_value_t)malloc(file.super.record.value_size);
			PLANCK_UNIT_ASSERT_TRUE(tc, err_ok 			== ff_query(&file,(ion_key_t)&i, value2));
			//build up expected value
			char str[10];
			sprintf(str,"%02i is key",i);
			PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *)value2, str);
			if (value2 != NULL)							//must free value after query
			{
				free(value2);
			}
		}
	}
	free(value);

	fclose(file.file_ptr);
	fremove(TEST_FILE);

}

/**
@brief		Tests that duplicate values are not allowed with write_concern
			is set for wc_insert_unique.

@details	Tests that duplicate values are not allowed with write_concern
			is set for wc_insert_unique.   Will generate error.

@param 		tc
				plank_unit_test_t
 */
void
test_flat_file_duplicate_insert_1(
	planck_unit_test_t	*tc
)
{
	ff_file_t file;									//create handler
	int i;

	initialize_flat_file_std_conditions(&file);

	//check to make sure that the write concern is set to wc_insert_unique (default)
	PLANCK_UNIT_ASSERT_TRUE(tc, wc_insert_unique == file.write_concern);

	//populate the map to only half capacity to make sure there is room
	for (i = 0; i<( STD_KV_SIZE/2); i++)
	{
		//build up the value
		char str[10];
		sprintf(str,"%02i is key",i);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok  	== ff_insert(&file,
													(ion_key_t)(&i),
													(ion_value_t)str));
	}

	/** and attempt to insert values with same key, which should fail and should
	return err_duplicate_key*/
	for (i = 0; i<( STD_KV_SIZE/2); i++)
	{
		//build up the value
		char str[10];
		sprintf(str,"%02i is key",i);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_duplicate_key  	== ff_insert(&file,
													(ion_key_t)(&i),
													(ion_value_t)str));
	}

	fclose(file.file_ptr);
	fremove(TEST_FILE);

}

/**
@brief		Tests that value updates are allowed with write_concern
			is set for wc_update.

@details	Tests that duplicate values are not allowed with write_concern
			is set for wc_update but will update the value.

@param 		tc
				plank_unit_test_t
 */
void
test_flat_file_duplicate_insert_2(
	planck_unit_test_t	*tc
)
{
	ff_file_t file;								//create handler for hashmap
	int i;

	initialize_flat_file_std_conditions(&file);

	//change write concern to allow up updates
	file.write_concern = wc_update;

	//check to make sure that the write concern is set to update
	PLANCK_UNIT_ASSERT_TRUE(tc, wc_update == file.write_concern);

	//populate the map to only half capacity to make sure there is room
	for (i = 0; i<(STD_KV_SIZE); i++)
	{
		//build up the value
		char str[10];
		sprintf(str,"%02i is key",i);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok  	== ff_insert(&file,
													(ion_key_t)(&i),
													(ion_value_t)str));
	}

	//check status of <K,V>
	for (i = 0; i<STD_KV_SIZE; i++)
	{
		ion_value_t value;
		value = (ion_value_t)malloc(file.super.record.value_size);

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok 	== ff_query(&file,(ion_key_t)&i, value));
		//build up expected value
		char str[10];
		sprintf(str,"%02i is key",i);
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *)value, str);
		if (value != NULL)							//must free value after query
		{
			free(value);
		}
	}

	/** and attempt to insert new values with same key*/
	for (i = 0; i<(STD_KV_SIZE); i++)
	{
		//build up the value
		char str[10];
		sprintf(str,"%02i is new",i);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok  	== ff_insert(&file,
													(ion_key_t)(&i),
													(ion_value_t)str));
	}

	//and check updated status of <K,V>
	for (i = 0; i<STD_KV_SIZE; i++)
	{
		ion_value_t value;
		value = (ion_value_t)malloc(file.super.record.value_size);

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok 		== ff_query(&file,
													(ion_key_t)&i,
													value));
		//build up expected value
		char str[10];
		sprintf(str,"%02i is new",i);
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *)value, str);
		if (value != NULL)							//must free value after query
		{
			free(value);
		}
	}
	fclose(file.file_ptr);
	fremove(TEST_FILE);
}

/**
@brief		Tests that values can be updated.

@param 		tc
				plank_unit_test_t
 */
void
test_flat_file_update_1(
	planck_unit_test_t	*tc
)
{
	ff_file_t file;								//create handler for hashmap
	int i;

	initialize_flat_file_std_conditions(&file);

	//change write concern to allow up updates
	file.write_concern = wc_insert_unique;

	//check to make sure that the write concern is set to wc_insert_unique (default)
	PLANCK_UNIT_ASSERT_TRUE(tc, wc_insert_unique == file.write_concern);

	//populate the map to only half capacity to make sure there is room
	for (i = 0; i<(STD_KV_SIZE); i++)
	{
		//build up the value
		char str[10];
		sprintf(str,"%02i is key",i);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok  	== ff_insert(&file,
													(ion_key_t)(&i),
													(ion_value_t)str));
	}

	//check status of <K,V>
	for (i = 0; i<STD_KV_SIZE; i++)
	{
		ion_value_t value;;
		value = (ion_value_t)malloc(file.super.record.value_size);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok 		== ff_query(&file,(ion_key_t)&i, value));
		//build up expected value
		char str[10];
		sprintf(str,"%02i is key",i);
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *)value, str);
		if (value != NULL)							//must free value after query
		{
			free(value);
		}
	}

	/** and update the values for the known keys */
	for (i = 0; i<(STD_KV_SIZE); i++)
	{
		//build up the value
		char str[10];
		sprintf(str,"%02i is new",i);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok  	== ff_update(&file,
													(ion_key_t)(&i),
													(ion_value_t)str));
	}

	//and check updated status of <K,V>
	for (i = 0; i<STD_KV_SIZE; i++)
	{
		ion_value_t value;
		value = (ion_value_t)malloc(file.super.record.value_size);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok 	== ff_query(&file,(ion_key_t)&i, value));
		//build up expected value
		char str[10];
		sprintf(str,"%02i is new",i);
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *)value, str);
		if (value != NULL)							//must free value after query
		{
			free(value);
		}
	}
	fclose(file.file_ptr);
	fremove(TEST_FILE);
}

/**
@brief		Tests that values will be ibnserted on update if they do not exist
 	 	 	in dictionary already.

@param 		tc
				plank_unit_test_t
 */
void
test_flat_file_update_2(
	planck_unit_test_t	*tc
)
{
	ff_file_t file;								//create handler for hashmap
	int i;

	initialize_flat_file_std_conditions(&file);

	//change write concern to allow up updates
	file.write_concern = wc_insert_unique;

	//check to make sure that the write concern is set to wc_insert_unique (default)
	PLANCK_UNIT_ASSERT_TRUE(tc, wc_insert_unique == file.write_concern);

	//populate the map to only half capacity to make sure there is room
	for (i = 0; i<(STD_KV_SIZE/2); i++)
	{
		//build up the value
		char str[10];
		sprintf(str,"%02i is key",i);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok  	== ff_insert(&file,
													(ion_key_t)(&i),
													(ion_value_t)str));
	}

	//check status of <K,V>
	for (i = 0; i<STD_KV_SIZE/2; i++)
	{
		ion_value_t value;
		value = (ion_value_t)malloc(file.super.record.value_size);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok 		== ff_query(&file,
													(ion_key_t)&i,
													value));
		//build up expected value
		char str[10];
		sprintf(str,"%02i is key",i);
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *)value, str);
		if (value != NULL)							//must free value after query
		{
			free(value);
		}
	}

	/** and update the values for the known keys */
	for (i = 0; i<(STD_KV_SIZE); i++)
	{
		//build up the value
		char str[10];
		sprintf(str,"%02i is new",i);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok  	== ff_update(&file,
													(ion_key_t)(&i),
													(ion_value_t)str));
	}

	//and check updated status of <K,V>
	for (i = 0; i<STD_KV_SIZE; i++)
	{
		ion_value_t value;
		value = (ion_value_t)malloc(file.super.record.value_size);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok 		== ff_query(&file,
													(ion_key_t)&i,
													value));
		//build up expected value
		char str[10];
		sprintf(str,"%02i is new",i);
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *)value, str);
		if (value != NULL)
			{
				free(value);
			}							//must free value after query
	}
	fclose(file.file_ptr);
	fremove(TEST_FILE);
}

/**
@brief		Tests that values can be deleted from dictionary with single value.

@param 		tc
				CuTeest
 */
void
test_flat_file_delete_1(
	planck_unit_test_t	*tc
)
{
	ff_file_t file;								//create handler for hashmap
	int i = 2;

	initialize_flat_file_std_conditions(&file);

	char str[10];
	sprintf(str,"%02i is key",i);
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok  	== ff_insert(&file,
												(ion_key_t)(&i),
												(ion_value_t)str));

	ion_status_t status;
	status = ff_delete(&file, (ion_key_t)(&i));
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok  	== status.err);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1	 		== status.count);
	//Check that value is not there
	ion_value_t value;
	value = (ion_value_t)malloc(file.super.record.value_size);
	PLANCK_UNIT_ASSERT_TRUE(tc, err_item_not_found
											== ff_query(&file,
												(ion_key_t)(&i),
												value));
	if (value != NULL)
	{
		free(value);
	}

	//Check that value can not be deleted if it is not there already
	status =  ff_delete(&file, (ion_key_t)(&i));
	PLANCK_UNIT_ASSERT_TRUE(tc, err_item_not_found
											== status.err);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0	 		== status.count);

	fclose(file.file_ptr);
	fremove(TEST_FILE);
}

/**
@brief		Tests that values can be deleted from dictionary with multiple
			values.

@details	Tests that values can be deleted from dictionary with multiple
			values.  Dictionary will have multiple values and only a single
			value will be deleted.  Checks to ensure that other values are
			undisturbed.

@param 		tc
				plank_unit_test_t
 */
void
test_flat_file_delete_2(
	planck_unit_test_t	*tc
)
{
	ff_file_t file;								//create handler
	int i, j;

	initialize_flat_file_std_conditions(&file);

	//populate the map
	for (i = 0; i<(STD_KV_SIZE); i++)
	{
		//build up the value
		char str[10];
		sprintf(str,"%02i is key",i);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok  == ff_insert(&file,
												(ion_key_t)(&i),
												(ion_value_t)str));
	}

	//check status of <K,V>
	for (i = 0; i<STD_KV_SIZE; i++)
	{
		ion_value_t value;
		value = (ion_value_t)malloc(file.super.record.value_size);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok 	== ff_query(&file,
												(ion_key_t)&i,
												value));
		//build up expected value
		char str[10];
		sprintf(str,"%02i is key",i);
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *)value, str);
		if (value != NULL)							//must free value after query
		{
			free(value);
		}
	}

	/** and update the values for the known keys */
	for (i = (STD_KV_SIZE - 1); i >= 0; i--)
	{
#if DEBUG
		printf("Deleting key: %i \n",i);
#endif
		ion_status_t status;
		status = ff_delete(&file, (ion_key_t)(&i));
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok  == status.err);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1  		== status.count);

		//Check that value is not there
		ion_value_t value;
		value = (ion_value_t)malloc(file.super.record.value_size);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_item_not_found
											== ff_query(&file,
												(ion_key_t)(&i),
												value));
		if (value != NULL)
		{
			free(value);
		}

		//and check that the rest of the values are still there
		for (j = 0; j<i; j++)
		{
			ion_value_t value;
			value = (ion_value_t)malloc(file.super.record.value_size);
			PLANCK_UNIT_ASSERT_TRUE(tc, err_ok
											== ff_query(&file,
												(ion_key_t)&j,
												value));
			//build up expected value
			char str[10];
			sprintf(str,"%02i is key",j);
			PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *)value, str);
			if (value != NULL)							//must free value after query
			{
				free(value);
			}
		}
	}

	//Check that all the values have been deleted
	for (i = 0; i<STD_KV_SIZE; i++)
	{
		ion_value_t value;
		value = (ion_value_t)malloc(file.super.record.value_size);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_item_not_found
											== ff_query(&file,
												(ion_key_t)&i,
												value));
		if (value != NULL)							//must free value after query
		{
			free(value);
		}
	}
	fclose(file.file_ptr);
	fremove(TEST_FILE);
}

planck_unit_suite_t*
flat_file_getsuite()
{
	planck_unit_suite_t *suite = planck_unit_new_suite();

	planck_unit_add_to_suite(suite, test_flat_file_initialize);
	planck_unit_add_to_suite(suite, test_flat_file_simple_insert);
	planck_unit_add_to_suite(suite, test_flat_file_simple_insert_and_query);
	planck_unit_add_to_suite(suite, test_flat_file_simple_delete);
	planck_unit_add_to_suite(suite, test_flat_file_duplicate_insert_1);
	planck_unit_add_to_suite(suite, test_flat_file_duplicate_insert_2);
	planck_unit_add_to_suite(suite, test_flat_file_update_1);
	planck_unit_add_to_suite(suite, test_flat_file_update_2);
	planck_unit_add_to_suite(suite, test_flat_file_delete_1);
	planck_unit_add_to_suite(suite, test_flat_file_delete_2);

	return suite;
}


void
runalltests_flat_file()
{
	planck_unit_suite_t		*suite	= flat_file_getsuite();

	planck_unit_run_suite(suite);
}