/*
This application isn't endorsed by Riot Games and doesn't reflect
the views or opinions of Riot Games or anyone officially involved
in producing or managing League of Legends. League of Legends and
Riot Games are trademarks or registered trademarks of Riot Games,
Inc. League of Legends © Riot Games, Inc.
*/

/*
This program analyzes any set of the static Black Market Brawlers matches.
It outputs data to 
	OUTPUT_DATA_FILE
	OUTPUT_BRAWLER_DATA_FILE

The data output is as follows
	//OUTPUT_DATA_FILE ONLY
	long long championId;
	long long razorfinsBought;
	long long ironbacksBought;
	long long plundercrabsBought;
	long long ocklepodsBought;

	//OUTPUT_DATA_FILE & OUTPUT_BRAWLER_DATA_FILE
	long long offensiveBuilds;
	long long defensiveBuilds;
	long long totalDamageTaken;
	long long totalDamageDealt;
	long long wins;
	long long picks;

	//OUTPUT_BRAWLER_DATA_FILE
	object items;
		"id" => (long long)buyCount;

in a json array of arrays, all data is in that order.

Warnings:
	This program may leak memory. Any function called to release memory causes memory corruption...
	This program is Organic*.
*/

#include <stdlib.h>
#include <stdio.h>

#include <windows.h>

#include <curl/curl.h>

#include "json.h"
#include "json-builder.h"

#define TEMP_DATA_FILE "C:\\bmb\\tempMatchData.json"         //Matches will be cURLed here
#define MATCH_ID_FILE "C:\\bmb\\matchIds.json"               //Must contain the matchIds
#define OUTPUT_DATA_FILE "C:\\bmb\\champData.json"           //Will contain the data collected about champions
#define OUTPUT_BRAWLER_DATA_FILE "C:\\bmb\\brawlerData.json" //Will contain the data collected about brawlers
#define ITEM_DATA_FILE "C:\\bmb\\items.json"                 //Must contain static item data
#define API_KEY "8e2ea4b4-79ef-48dc-8560-84d9593f6ba8"

//global vars
#undef DO_NOT_DEFINE_
#ifndef DO_NOT_DEFINE_
FILE *matchDataFileStream;
FILE *matchIdFileStream;
FILE *outputDataFileStream;
FILE *outputBrawlerDataFileStream;
FILE *tempFileStream;

json_value *matchIdArray;
json_value *matchData;
json_value *itemDataArray;

json_value *participants;
json_value *timeline;

char matchDataURI[256];

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp);
void saveData();
int* stat_tell(char desc[]);

int brawlerItemIds[4] = { 3611, 3612, 3613, 3614 };
char brawlerNames[4][12] = { "Razorfin   ", "Ironback   ", "Plundercrab", "Ocklepod   " };

int blackMarketItemIds[23] = {
	3742, // Dead Man's Plate
	3911, // Martyr's Gambit
	3430, // Rite of Ruin
	3744, // Staff of Flowing Water
	3829, // Trickster's Glass
	3652, // Typhoon Claws
	3433, // Lost Chapter
	3150, // Mirage Blade
	3844, // Murksphere
	3431, // Netherstride Grimoire
	3841, // Swindler's Orb
	3840, // Globe of Trust
	3434, // Pox Arcana
	3745, // Puppeteer
	3924, // Flesheater
	
	1335,/*teleport*/
	1336,
	1337,
	1338,
	1339,
	1340,
	1341,
	3245 /*tropelet*/
};

int brawlerBought[10]          = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
int championPlayed[10]         = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
int wonGame[10]                = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
long long dmgDealt[10]         = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
long long dmgTaken[10]         = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
int itemsBought[10][6]         = {
	{ -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1, -1 }
};

FILE *caughtItemData;

int currentMatchId = 0;

//builds = items... participant has 6 builds @EOG
struct champData {
	long long razorfinsBought;
	long long ironbacksBought;
	long long plundercrabsBought;
	long long ocklepodsBought;

	long long offensiveBuilds;
	long long defensiveBuilds;
	long long totalDamageTaken;
	long long totalDamageDealt;
	long long wins;
	long long picks;
};

struct champData outputData[1024];

struct brawlerData {
	long long offensiveBuilds;
	long long defensiveBuilds;
	long long totalDamageTakenByBuyers;
	long long totalDamageDealtByBuyers;
	long long wins;
	long long picks;
	long long itemsBought[8192];
};

struct brawlerData outputBrawlerData[4];
#endif

int main(int argc, char* argv[]) {

	puts("Program start.");

	atexit(saveData);

	//open output files (this is done now so there would be an error at the start instead of in the middle of the program)
	if (1) {
		//delete + open file for raw data writing
		outputDataFileStream = fopen(OUTPUT_DATA_FILE, "wb");

		if (outputDataFileStream == NULL) {
			puts("Could not open \"" OUTPUT_DATA_FILE "\"!");
			system("pause");
			return -1;
		}
		
		//delete + open file for raw data writing
		outputBrawlerDataFileStream = fopen(OUTPUT_BRAWLER_DATA_FILE, "wb");

		if (outputBrawlerDataFileStream == NULL) {
			puts("Could not open \"" OUTPUT_BRAWLER_DATA_FILE "\"!");
			system("pause");
			return -1;
		}
	}

	//parse static data files
	if (1) {
		char *matchIdString = 0;
		long length;
		matchIdFileStream = fopen(MATCH_ID_FILE, "rb");

		if (matchIdFileStream == NULL) {
			puts("Could not open \"" MATCH_ID_FILE "\"!");
			system("pause");
			return -1;
		}

		fseek(matchIdFileStream, 0, SEEK_END);
		length = ftell(matchIdFileStream);
		fseek(matchIdFileStream, 0, SEEK_SET);
		matchIdString = malloc(length);
		fread(matchIdString, 1, length, matchIdFileStream);
		fclose(matchIdFileStream);

		matchIdArray = json_parse(
			matchIdString,
			length
			);
		free(matchIdString);
		char *itemDataString = 0;
		length = 0;
		caughtItemData = fopen(ITEM_DATA_FILE, "rb");

		if (caughtItemData == NULL) {
			puts("Could not open \"" ITEM_DATA_FILE "\"!");
			system("pause");
			return -1;
		}

		fseek(caughtItemData, 0, SEEK_END);
		length = ftell(caughtItemData);
		fseek(caughtItemData, 0, SEEK_SET);
		itemDataString = malloc(length);
		fread(itemDataString, 1, length, caughtItemData);
		fclose(caughtItemData);

		itemDataArray = json_parse(
			itemDataString,
			length
			);

		if (itemDataArray == NULL) {
			puts("dat err");
			system("pause");
			return -1;
		}
		free(itemDataString);
	}

	PROG_BEGIN:
	printf("Working... %d\n", currentMatchId);

	//init
	if (1) {
		memset(brawlerBought, -1, 10);
		memset(championPlayed, -1, 10);
		memset(wonGame, -1, 10);
		memset(dmgDealt, -1, 10);
		memset(dmgTaken, -1, 10);
		for (int i = 0; i < 10; i++) 
			memset(itemsBought[i], -1, 10);
	}

	//get matchId and form riot api data URL
	if (1) {
		//get match id
		long long matchId = 0;
		matchId = matchIdArray->u.array.values[currentMatchId]->u.integer;

		//turn it into a string
		char matchIdString[32];
		snprintf(matchIdString, 32, "%d", matchId);
		//printf("id: / %s /\n", matchIdString);

		//form URL
		strcpy(matchDataURI, "https://na.api.pvp.net/api/lol/na/v2.2/match/");
		strcat(matchDataURI, matchIdString);
		strcat(matchDataURI, "?includeTimeline=true&api_key=" API_KEY);
	}

	//delete the temp match data file and open it for appendage
	if (1) {
		fopen_s(&matchDataFileStream, TEMP_DATA_FILE, "w"); //delete
		if (matchDataFileStream == NULL) {
			puts("Could not open \"" TEMP_DATA_FILE "\"!");
			system("pause");
			return -1;
		}
		freopen_s(&matchDataFileStream, TEMP_DATA_FILE, "ab", matchDataFileStream); //append raw binary data
	}

	//start curl and download the data into the temp match data file
	if (1) {
		tempFileStream = matchDataFileStream;
		CURL *cURLHandle = curl_easy_init();

		curl_easy_setopt(cURLHandle, CURLOPT_URL, matchDataURI);
		curl_easy_setopt(cURLHandle, CURLOPT_WRITEFUNCTION, write_data);

		curl_easy_setopt(cURLHandle, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_setopt(cURLHandle, CURLOPT_SSL_VERIFYPEER, 0L);

		//curl_easy_setopt(cURLHandle, CURLOPT_VERBOSE, 1L); //for debugging

		CURLcode success = curl_easy_perform(cURLHandle);
		curl_easy_cleanup(cURLHandle);

		if (success != CURLE_OK) {
			printf("cURL error\n");
			system("pause");
			return -1;
		}//else printf("Downloaded \n\t\"%s\"\n\nInto \n\t\"" TEMP_DATA_FILE "\"\n", matchDataURI);
	}

	//parse the file that was just downloaded
	if (1) {
		char *matchDataString = 0;
		long length;
		matchDataFileStream = freopen(TEMP_DATA_FILE, "rb", matchDataFileStream);

		if (matchDataFileStream == NULL) {
			puts("Could not open \"" TEMP_DATA_FILE "\"!\n");
			system("pause");
			return -1;
		}

		fseek(matchDataFileStream, 0, SEEK_END);
		length = ftell(matchDataFileStream);
		fseek(matchDataFileStream, 0, SEEK_SET);
		matchDataString = malloc(length);
		fread(matchDataString, 1, length, matchDataFileStream);
		fclose(matchDataFileStream);

		if (strstr(matchDataString, "Rate limit exceeded")) {
			puts("Request limit exceeded...\n Retrying in 5 secs...\n");
			Sleep(5000);
			goto PROG_BEGIN;
		}

		matchData = json_parse(
			matchDataString,
			length
			);

		if (matchData == NULL) {
			printf("%s\n Error: no match data---\n");
			if (strstr(matchDataString, "HTTP ERROR 429")) {
				puts("Request limit exceeded...\n Retrying in 10 secs...\n");
				puts("10");
				Sleep(2000);
				puts("8");
				Sleep(2000);
				puts("6");
				Sleep(2000);
				puts("4");
				Sleep(2000);
				puts("2");
				Sleep(1000);
				puts("1");
				Sleep(1000);
				goto PROG_BEGIN;
			}else {
				printf("Unknown error. Retrying program after match #%d", currentMatchId);
				goto PROG_BEGIN;
			}
		}
		free(matchDataString);
	}

	//find the participant data and the timeline
	if (1) {
		for (int i = 0; i < matchData->u.object.length; i++) {
			if (!strcmp(matchData->u.object.values[i].name, "participants")) {
				participants = matchData->u.object.values[i].value;
				//printf("participants located @ %d\n", i);
				break;
			}
		}
		if (participants == NULL) {
			puts("Could not find participants!");
			system("pause");
			return -1;
		}

		for (int i = 0; i < matchData->u.object.length; i++) {
			if (!strcmp(matchData->u.object.values[i].name, "timeline")) {
				timeline = matchData->u.object.values[i].value->u.object.values[0].value;
				//printf("timeline located @ %d\n", i);
				break;
			}
		}
		if (timeline == NULL) {
			puts("Could not find timeline!");
			system("pause");
			return -1;
		}
	}

	//find what champion each participant played/if they won or not/which items they ended with
	if (1) {
		if (participants->u.array.length != 10) {
			printf("10 players were not present in match #%d! %d were. Retrying match...", currentMatchId, participants->u.array.length);
			goto PROG_BEGIN;
		}

		//for each participant
		for (int i = 0; i < 10; i++) {

			//for every piece of data about that participant
			for (int j = 0; j < participants->u.array.values[i]->u.object.length; j++) {
				//if this piece of data is the champion ID, save it
				if (!strcmp(participants->u.array.values[i]->u.object.values[j].name, "championId")) {
					//                  participants->      participant->        championId.value
					championPlayed[i] = participants->u.array.values[i]->u.object.values[j].value->u.integer;
					break; //next participant
				}
			}

			//for every piece of data about that participant
			for (int j = 0; j < participants->u.array.values[i]->u.object.length; j++) {
				//if this piece of data is about stats,
				if (!strcmp(participants->u.array.values[i]->u.object.values[j].name, "stats")) {
					for (int k = 0; k < participants->u.array.values[i]->u.object.values[j].value->u.object.length; k++) {
						//          participants        participant               stats                      stat.name
						if (!strcmp(participants->u.array.values[i]->u.object.values[j].value->u.object.values[k].name, "item0")) {
							
							//the next 5 are the other items, save all
							for (int l = 0; l < 6; l++) 
								itemsBought[i][l] = participants->u.array.values[i]->u.object.values[j].value->u.object.values[k + l].value->u.integer;
						}

					}
				}
			}

			//for every piece of data about that participant
			for (int j = 0; j < participants->u.array.values[i]->u.object.length; j++) {
				//if this piece of data is about stats,
				if (!strcmp(participants->u.array.values[i]->u.object.values[j].name, "stats")) {
					for (int k = 0; k < participants->u.array.values[i]->u.object.values[j].value->u.object.length; k++) {
						//          participants        participant               stats                      stat.name
						if (!strcmp(participants->u.array.values[i]->u.object.values[j].value->u.object.values[k].name, "totalDamageDealt")) {

							dmgDealt[i] = participants->u.array.values[i]->u.object.values[j].value->u.object.values[k].value->u.integer;
						}
						
					}
				}
			}

			//for every piece of data about that participant
			for (int j = 0; j < participants->u.array.values[i]->u.object.length; j++) {
				//if this piece of data is about stats,
				if (!strcmp(participants->u.array.values[i]->u.object.values[j].name, "stats")) {
					for (int k = 0; k < participants->u.array.values[i]->u.object.values[j].value->u.object.length; k++) {
						//          participants        participant               stats                      stat.name
						if (!strcmp(participants->u.array.values[i]->u.object.values[j].value->u.object.values[k].name, "totalDamageTaken")) {

							dmgTaken[i] = participants->u.array.values[i]->u.object.values[j].value->u.object.values[k].value->u.integer;
						}

					}
				}
			}

			//for every piece of data about that participant
			for (int j = 0; j < participants->u.array.values[i]->u.object.length; j++) {
				//if this piece of data is about items,
				if (!strcmp(participants->u.array.values[i]->u.object.values[j].name, "stats")) {
					for (int k = 0; k < participants->u.array.values[i]->u.object.values[j].value->u.object.length; k++) {
						//          participants        participant               stats                      stat.name
						if (!strcmp(participants->u.array.values[i]->u.object.values[j].value->u.object.values[k].name, "winner")) {
							wonGame[i] = participants->u.array.values[i]->u.object.values[j].value->u.object.values[k].value->u.boolean;
						}

					}
				}
			}

		}
	}

	//find which brawler each participant bought (searches throught every item bought in the game bc brawlers dont go into inventory)
	if (1) {
		//for every set of info in timeline
		for (int i = 0; i < timeline->u.array.length; i++) {

			//for every piece of info in this set, preferably Frame
			for (int j = 0; j < timeline->u.array.values[i]->u.object.length; j++) {

				//if this info set is Frame, it will have the data set "events". check for that
				if (!strcmp(timeline->u.array.values[i]->u.object.values[j].name, "events")) {
					json_value *event = timeline->u.array.values[i]->u.object.values[j].value;

					//for every data packet from the set of events (in the set Frame, (in the set timeline))
					for (int k = 0; k < event->u.array.length; k++) {

						//data for the current participant that im looking at
						//i dont know who the participant is yet, but that data will be collected, and noted
						int itemBought = -1; //more of a status than an ID
						int brawlerBought_ = -1; //ID
						int currentParticipant = -1; //ID

						//collect neccesary data
						if (1) {
							//for every descriptor in this data packet
							for (int l = 0; l < event->u.array.values[k]->u.object.length; l++) {
								//if we're looking at the event type
								if (!strcmp(event->u.array.values[k]->u.object.values[l].name, "eventType")) {

									if (!strcmp(event->u.array.values[k]->u.object.values[l].value->u.string.ptr, "ITEM_PURCHASED"))
										itemBought = 1;

									if (!strcmp(event->u.array.values[k]->u.object.values[l].value->u.string.ptr, "ITEM_SOLD"))
										itemBought = 2;

									if (!strcmp(event->u.array.values[k]->u.object.values[l].value->u.string.ptr, "ITEM_UNDO"))
										itemBought = 3;

									if (!strcmp(event->u.array.values[k]->u.object.values[l].value->u.string.ptr, "ITEM_DESTROYED"))
										itemBought = 4;

								}
							}

							//for every descriptor in this data packet
							for (int l = 0; l < event->u.array.values[k]->u.object.length; l++) {

								//if we're looking at the participant id
								if (!strcmp(event->u.array.values[k]->u.object.values[l].name, "participantId")) {

									currentParticipant = event->u.array.values[k]->u.object.values[l].value->u.integer - 1;
								}
							}

							//for every descriptor in this data packet
							for (int l = 0; l < event->u.array.values[k]->u.object.length; l++) {

								//if were looking at the item id
								if (!strcmp(event->u.array.values[k]->u.object.values[l].name, "itemId")) {

									//if the itemid is one of a brawler, note that
									for (int m = 0; m < 4; m++) {
										if (event->u.array.values[k]->u.object.values[l].value->u.integer == brawlerItemIds[m]) {
											brawlerBought_ = m;
										}
									}
								}
							}
						}

						//if a brawler was bought
						if (brawlerBought_ != -1) {
							switch (itemBought) {
							case 1: //brawler bought
								brawlerBought[currentParticipant] = brawlerBought_;
								break;
							case 2: //brawler sold
								brawlerBought[currentParticipant] = -1;
								printf("participant %d sold  a %s\n", currentParticipant, brawlerNames[brawlerBought_]);
								break;
							case 3: //brawler undone
								brawlerBought[currentParticipant] = -1;
								printf("participant %d undid a %s\n", currentParticipant, brawlerNames[brawlerBought_]);
								break;
							case 4: //brawler destroyed: this happens when they buy it so no print...
								//brawlerBought[currentParticipant] = -1;
								//printf("participant %d destroyed a %s\n", currentParticipant, brawlerNames[brawlerBought_]);
								break;
							default:
								break;
							}
						}
						//processed one event in "events"
					}
					//processed "events", in Frame
				}
				//processed something that may or may not have been "events", in Frame
			}
			//processed all data in this data set
		}
		//processed all data sets in the timeline
		//ignored all but Frame, which contains "events"
	}

	//update data structure
	for (int i = 0; i < 10; i++) {
		//printf("participant #%d played champion #%d and bought a %s\n", i, championPlayed[i], brawlerNames[brawlerBought[i]]);

		outputData[championPlayed[i]].wins += wonGame[i];
		outputData[championPlayed[i]].totalDamageDealt += dmgDealt[i];
		outputData[championPlayed[i]].totalDamageTaken += dmgTaken[i];
		outputData[championPlayed[i]].picks++;


		//analyze items
		for (int j = 0; j < 6; j++) {

			if (itemsBought[i][j] < 8) continue;
			char itemIdString[32];
			//form url
			if (1) {
				//get item id
				long long itemId = itemsBought[i][j];

				//turn it into a string
				snprintf(itemIdString, 32, "%d", itemId);
				//printf("id: / %s /\n", matchIdString);
			}
			
			long long length = 0;
			json_value *itemObject;
			char* item = 0;
			int found = 0;

			//json-parser
			if (1) {

				//SEARCH FOR ITEM AND SET itemObject TO THAT ITEM'S ARRAY

				for (int k = 0; k < itemDataArray->u.object.length; k++) {
					if (!strcmp(itemDataArray->u.object.values[k].name, "data")) {
						for (int l = 0; l < itemDataArray->u.object.values[k].value->u.object.length; l++) {
							if (!strcmp(itemDataArray->u.object.values[k].value->u.object.values[l].name, itemIdString)) {
								itemObject = itemDataArray->u.object.values[k].value->u.object.values[l].value;
								found = 1;
								break;
							}
						}
					}
					if (found)break;
				}

				if (!found) {
					printf("Error: no item data.\n");
					system("pause");
					return -1;
				}
			}

			//find the desc and analyze it for its stats
			int descIndex = 0;
			for (int k = 0; k < itemObject->u.object.length; k++) {
				if (!strcmp(itemObject->u.object.values[k].name, "description")) {
					descIndex = k;
					break;
				}
			}

			int *stat_type = stat_tell(itemObject->u.object.values[descIndex].value->u.string.ptr);

			//update info
			outputData[championPlayed[i]].offensiveBuilds += stat_type[0];
			outputData[championPlayed[i]].defensiveBuilds += stat_type[1];

			if (brawlerBought[i] > -1 && brawlerBought[i] < 4) {
				outputBrawlerData[brawlerBought[i]].defensiveBuilds += stat_type[0];
				outputBrawlerData[brawlerBought[i]].offensiveBuilds += stat_type[1];
			}
			
		}
		
		switch (brawlerBought[i]) {
		case 0:
			outputData[championPlayed[i]].razorfinsBought++;
			break;
		case 1:
			outputData[championPlayed[i]].ironbacksBought++;
			break;
		case 2:
			outputData[championPlayed[i]].plundercrabsBought++;
			break;
		case 3:
			outputData[championPlayed[i]].ocklepodsBought++;
			break;
		default:
			printf("data anomaly: participant #%d bought brawler #%d (-1 == no brawler)\n", i, brawlerBought[i]);
			continue;
		}

		outputBrawlerData[brawlerBought[i]].wins += wonGame[i];
		outputBrawlerData[brawlerBought[i]].totalDamageDealtByBuyers += dmgDealt[i];
		outputBrawlerData[brawlerBought[i]].totalDamageTakenByBuyers += dmgTaken[i];
		outputBrawlerData[brawlerBought[i]].picks++;
		for (int j = 0; j < 6; j++)
			outputBrawlerData[brawlerBought[i]].itemsBought[itemsBought[i][j]]++;
		
	}
	
	//maintenance before starting again or exiting
	if (1) {
		//json_value_free(matchData);
		++currentMatchId;
	}
	
	//if all data has been processed, exit
	if (currentMatchId > 999) {
		exit(0);
	}

	goto PROG_BEGIN;

	return 0;
}

//downloading function
size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp) {

	fwrite(buffer, size, nmemb, tempFileStream);

	return nmemb;
}

//exit funtion
void saveData() {
	
	printf("Saving data @ %d...\n", currentMatchId);

	//make new arrays to hold the data
	json_value *outputArray = json_array_new(0);
	json_value **structures = malloc(1024);

	//TODO: Save brawler data into BRAWLER_DATA_FILE
	json_value *outputBrawlerArray = json_array_new(0);
	json_value *(brawlerStructs[4]);

	//for every champion that may or may not have had data stored for
	for (int i = 0; i < 1024; i++) {
		//if this champion had any data stored for it (at least one brawler has been bought on this champ)
		if (outputData[i].razorfinsBought    +
			outputData[i].ironbacksBought    +
			outputData[i].plundercrabsBought +
			outputData[i].ocklepodsBought > 0) {

			//make it a new array
			structures[i] = json_array_new(11);

			//push the appropriate data into it
			json_array_push(structures[i], json_integer_new(i));
			json_array_push(structures[i], json_integer_new(outputData[i].razorfinsBought));
			json_array_push(structures[i], json_integer_new(outputData[i].ironbacksBought));
			json_array_push(structures[i], json_integer_new(outputData[i].plundercrabsBought));
			json_array_push(structures[i], json_integer_new(outputData[i].ocklepodsBought));

			json_array_push(structures[i], json_integer_new(outputData[i].offensiveBuilds));
			json_array_push(structures[i], json_integer_new(outputData[i].defensiveBuilds));

			json_array_push(structures[i], json_integer_new(outputData[i].totalDamageTaken));
			json_array_push(structures[i], json_integer_new(outputData[i].totalDamageDealt));

			json_array_push(structures[i], json_integer_new(outputData[i].wins));
			json_array_push(structures[i], json_integer_new(outputData[i].picks));

			//add it to the output array
			json_array_push(outputArray, structures[i]);
		}
	}
	
	//for every brawler
	for (int i = 0; i < 4; i++) {
		//make it a new array
		brawlerStructs[i] = json_array_new(6);

		//push the appropriate data into it
		json_array_push(brawlerStructs[i], json_integer_new(outputBrawlerData[i].offensiveBuilds));
		json_array_push(brawlerStructs[i], json_integer_new(outputBrawlerData[i].defensiveBuilds));

		json_array_push(brawlerStructs[i], json_integer_new(outputBrawlerData[i].totalDamageTakenByBuyers));
		json_array_push(brawlerStructs[i], json_integer_new(outputBrawlerData[i].totalDamageDealtByBuyers));

		json_array_push(brawlerStructs[i], json_integer_new(outputBrawlerData[i].wins));
		json_array_push(brawlerStructs[i], json_integer_new(outputBrawlerData[i].picks));

		json_value *boughtObject = json_object_new(0);

		for (int j = 0; j < 8192; j++) {
			if (outputBrawlerData[i].itemsBought[j] < 1) continue;
			char itemName[63];
			snprintf(itemName, 62, "%d", j);

			json_object_push(boughtObject, itemName, json_integer_new(outputBrawlerData[i].itemsBought[j]));
		}

		json_array_push(brawlerStructs[i], boughtObject);

		//add it to the output array
		json_array_push(outputBrawlerArray, brawlerStructs[i]);
	}

	json_serialize_opts opts;
	opts.mode = json_serialize_mode_packed;

	int length = json_measure_ex(outputArray, opts);
	char *outputJSONString = malloc(length);

	json_serialize_ex(outputJSONString, outputArray, opts);

	fwrite(outputJSONString, sizeof(char), length, outputDataFileStream);


	length = json_measure_ex(outputBrawlerArray, opts);
	char *outputJSONString_brawlers = malloc(length);

	json_serialize_ex(outputJSONString_brawlers, outputBrawlerArray, opts);

	fwrite(outputJSONString_brawlers, sizeof(char), length, outputBrawlerDataFileStream);

	fcloseall();

	//json_builder_free(outputArray);
	//json_builder_free(outputBrawlerArray);
	json_value_free(matchIdArray);
	return;
}

int* stat_tell(char desc[]) {
	//statType[0] - offense y/n
	//statType[1] - defense y/n
	int statType[2] = {0, 0};

	//if the string contains an offensive keyword
	char* index = strstr(desc, "Ability Power");

	if(index==NULL)index = strstr(desc, "Attack Damage");
	if(index==NULL)index = strstr(desc, "Attack Speed");
	if(index==NULL)index = strstr(desc, "Critical Strike Chance");
	if(index==NULL)index = strstr(desc, "Penetration");

	//set that
	if(index!=NULL)statType[0] = 1;

	//if the string contains a defensive keyword
	index = strstr(desc, "Health");
	if(index == NULL && strstr(desc, "Armor Penetration") == NULL) index = strstr(desc, "Armor");
	if(index == NULL)index = strstr(desc, "Magic Resist");

	//set that
	if (index != NULL)statType[1] = 1;

	return statType;
}

//*These statements have not been evaluated by the Food and Drug Administration.
// This product is not intended to diagnose, treat, cure or prevent any disease.