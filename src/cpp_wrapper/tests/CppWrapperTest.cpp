/******************************************************************************/
/**
@file		CppWrapperTest.cpp
@author		Dana Klamut
@brief		Test class exemplifying uses for Dictionary.h.
*/
/******************************************************************************/

#include <iostream>

#include "../../dictionary/ion_master_table.h"
#include "../MasterTable.h"

using namespace std;

int main() {
	/** The following is example test code for using Dictionary.h */

    Dictionary<int> *btree = new BppTree<int>(key_type_numeric_signed, sizeof(int), 10);

	printf("Insert %d [%s]\n", 3, "hello!");
	string s = "hello!";
	btree->insert(3, s);

	string buf = btree->get<string>(3);
	cout << "\nRetrieve key 3 and got back " << buf << endl;

	cout << "\nUpdating value of key 3 to hi!" << endl;

	string t = "hi!";
	btree->update(3, t);

	string buf2 = btree->get<string>(3);
	cout << "\nRetrieve key 3 after update and got back " << buf2 << endl;

	cout << "\nDelete key 3" << endl;
	btree->deleteRecord(3);

	cout << "\nTry and retrieve key 3 after deletion" << endl;

	string buf3 = btree->get<string>(3);

	cout << "\nRetrieve key 3 and got back " << buf3 << endl;

	printf("\nInsert %d [%s]\n", 3, "test1!");
	string u = "test1!";
	btree->insert(3, u);

	printf("Insert %d [%s]\n", 3, "test2!");
	string v = "test2!";
	btree->insert(3, v);

	printf("Insert %d [%s]\n", 4, "test3!");
	string w = "test3!";
	btree->insert(4, w);

	cout << "\nTesting equality query on key 3: " << endl;
	dict_cursor_t *cursor1 = btree->equality(3);
	ion_record_t rec11 = btree->getRecord(cursor1);
	int key11 = *((int*)rec11.key);
	cout << "Key: " << key11 << " Value: " << rec11.value << endl;
	ion_record_t rec12 = btree->getRecord(cursor1);
	int key12 = *((int*)rec12.key);
	cout << "Key: " << key12 << " Value: " << rec12.value << endl;
	ion_record_t rec13 = btree->getRecord(cursor1);
	int key13 = *((int*)rec13.key);
	cout << "Key: " << key13 << " Value: " << rec13.value << endl;

	cout << "\nTesting all records query: " << endl;
	dict_cursor_t *cursor2 = btree->allRecords();
	ion_record_t rec21 = btree->getRecord(cursor2);
	int key21 = *((int*)rec21.key);
	cout << "Key: " << key21 << " Value: " << rec21.value << endl;
	ion_record_t rec22 = btree->getRecord(cursor2);
	int key22 = *((int*)rec22.key);
	cout << "Key: " << key22 << " Value: " << rec22.value << endl;
	ion_record_t rec23 = btree->getRecord(cursor2);
	int key23 = *((int*)rec23.key);
	cout << "Key: " << key23 << " Value: " << rec23.value << endl;
	ion_record_t rec24 = btree->getRecord(cursor2);
	int key24 = *((int*)rec24.key);
	cout << "Key: " << key24 << " Value: " << rec24.value << endl;

	cout << "\nTesting range query: 2<=key<=3 " << endl;
	dict_cursor_t *cursor3 = btree->range(2, 3);
	ion_record_t rec31 = btree->getRecord(cursor3);
	int key31 = *((int*)rec31.key);
	cout << "Key: " << key31 << " Value: " << rec31.value << endl;
	ion_record_t rec32 = btree->getRecord(cursor3);
	int key32 = *((int*)rec32.key);
	cout << "Key: " << key32 << " Value: " << rec32.value << endl;
	ion_record_t rec33 = btree->getRecord(cursor3);
	int key33 = *((int*)rec33.key);
	cout << "Key: " << key33 << " Value: " << rec33.value << endl;
	cout << "\nEnd cpp wrapper test." << endl;

	btree->destroy();
	delete btree;
}