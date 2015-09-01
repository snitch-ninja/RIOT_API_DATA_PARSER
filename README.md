# RIOT_API_DATA_PARSER
This Visual Studio 2015 C Program is what I created to aggregate data and statistics from matches. It loops through 1000 matches in a match repository, like the Black Market Brawler match repos that were given out for the API challenge, and finds/saves all the info it needs on each loop.
<br />
<code>
/* The data output is as follows;
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

in a json array of arrays, all data is in that order. */
</code>
<br /><br />
See the comments in <code>&lt;main.c&gt;</code> for a more detailed explaination; collapsing the <code>if(1){}</code>'s and top level <code>for(...){}</code> loops inside <code>int main(...){}</code> provides a more readable experience.

I have changed my API key, so the one inside <code>&lt;main.c&gt;</code> does not work.
