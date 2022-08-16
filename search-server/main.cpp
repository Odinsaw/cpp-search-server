// -------- Начало модульных тестов поисковой системы ----------

// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
	const int doc_id = 42;
	const string content = "cat in the city"s;
	const vector<int> ratings = { 1, 2, 3 };
	// Сначала убеждаемся, что поиск слова, не входящего в список стоп-слов,
	// находит нужный документ
	{
		SearchServer server;
		server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
		const auto found_docs = server.FindTopDocuments("in"s);
		ASSERT_EQUAL(found_docs.size(), 1);
		const Document & doc0 = found_docs[0];
		ASSERT_EQUAL(doc0.id, doc_id);
	}

	// Затем убеждаемся, что поиск этого же слова, входящего в список стоп-слов,
	// возвращает пустой результат
	{
		SearchServer server;
		server.SetStopWords("in the"s);
		server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
		ASSERT(server.FindTopDocuments("in"s).empty());
	}
}

/*
Разместите код остальных тестов здесь
*/
void TestMinusWords() {
	const int doc_id = 14;
	const string content = "cat in the city"s;
	const vector<int> ratings = { 1, 2, 3 };

	{
		SearchServer server;
		server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
		const auto found_docs = server.FindTopDocuments("cat"s);
		ASSERT_EQUAL(found_docs.size(),1); //поиск без минус слов находит документ
		const Document & doc0 = found_docs[0];
		ASSERT_EQUAL(doc0.id, doc_id); 
	}
	{
		SearchServer server;
		server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
		ASSERT_HINT(server.FindTopDocuments("cat -city"s).empty(),"Minus words are not working!"s); //поиск с минус словом пропускает документ
	}
}
void TestMatchWords() {
	const int doc_id = 14;
	const string content = "cat in the city"s;
	const vector<int> ratings = { 1, 2, 3 };
	{
		SearchServer server;
		server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
		const auto matched_words = server.MatchDocument("dog from a town", doc_id);
		ASSERT(get<0>(matched_words).empty()); //документ не содержит слов из запроса
	}
	{
		SearchServer server;
		server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
		const auto matched_words = server.MatchDocument("cat from a city", doc_id);
		const vector<string> w = { "cat"s, "city"s };
		ASSERT(get<0>(matched_words) == w); //функция возвращает совпадающие слова из запроса
		ASSERT_HINT(get<1>(matched_words) == DocumentStatus::ACTUAL, "Match Words returns wrong status!"s);//функция провильно возвращает статус документа
	}
	{
		SearchServer server;
		server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
		const auto matched_words = server.MatchDocument("city -cat", doc_id);
		const vector<string> w = { "cat"s, "city"s };
		ASSERT_HINT(get<0>(matched_words).empty(), "Minus Words are not working!"s);//функция правильно обрабатывает минус слова
	}
}
void TestRelevanceAndRating() {
    //задаем набор документов с известными параметрами:
	const int doc_id_1 = 14, doc_id_2 = 66, doc_id_3 = 54;
	const string doc1 = "a colorful parrot with green wingsand red tail is lost"s,
		doc2 = "a white cat with long furry tail is found near the red square"s,
		doc3 = "a grey hound with black ears is found at the railway station",
		query = "white cat long tail"s,
		stop_w = "is are was a an in the with near at"s;
	const vector<int> ratings1 = { 1, 2, 3 }, ratings2 = { 2, 3, 4 }, ratings3 = { 2, 2, 4 };
	SearchServer server;
	server.SetStopWords(stop_w);
	server.AddDocument(doc_id_1, doc1, DocumentStatus::ACTUAL, ratings1);
	server.AddDocument(doc_id_2, doc2, DocumentStatus::ACTUAL, ratings2);
	server.AddDocument(doc_id_3, doc3, DocumentStatus::ACTUAL, ratings3);
    //проверяем, что функция правильно возвращает заранее известные релевантность (в пределах погрешности) и рейтинг для документа с соответствующим id
	ASSERT(abs(server.FindTopDocuments(query)[0].relevance - 0.462663) <= 10e-6 && server.FindTopDocuments(query)[0].id == 66);
	ASSERT(abs(server.FindTopDocuments(query)[1].relevance - 0.0579236) <= 10e-6 && server.FindTopDocuments(query)[1].id == 14);
	ASSERT_EQUAL_HINT(server.FindTopDocuments(query)[0].rating, 3,"Wrong rating!"s);
	ASSERT_EQUAL_HINT(server.FindTopDocuments(query)[1].rating, 2,"Wrong rating!"s);
}
void TestStatusAndSelector() {
	const int doc_id_1 = 14, doc_id_2 = 66, doc_id_3 = 54, doc_id_4 = 78;
	const string doc1 = "a colorful parrot with green wingsand red tail is lost"s,
		doc2 = "a white cat with long furry tail is found near the red square"s,
		doc3 = "a grey hound with black ears is found at the railway station",
		doc4 = "black horse with a tail",
		query = "white cat long tail and black ears"s,
		stop_w = "is are was a an in the with near at"s;
	const vector<int> ratings1 = { 1, 2, 3 }, ratings2 = { 2, 3, 4 }, ratings3 = { 2, 3, 4 }, ratings4 = { 4, 2, 4 };
	SearchServer server;
	server.SetStopWords(stop_w);
	server.AddDocument(doc_id_1, doc1, DocumentStatus::ACTUAL, ratings1);
	server.AddDocument(doc_id_2, doc2, DocumentStatus::IRRELEVANT, ratings2);
	server.AddDocument(doc_id_3, doc3, DocumentStatus::BANNED, ratings3);
	server.AddDocument(doc_id_4, doc4, DocumentStatus::REMOVED, ratings4);
    //проверяем, что поиск правильно обрабатывает статус
	ASSERT(server.FindTopDocuments(query, DocumentStatus::ACTUAL)[0].id == 14 && server.FindTopDocuments(query, DocumentStatus::ACTUAL).size() == 1);
	ASSERT(server.FindTopDocuments(query, DocumentStatus::IRRELEVANT)[0].id == 66 && server.FindTopDocuments(query, DocumentStatus::IRRELEVANT).size() == 1);
	ASSERT(server.FindTopDocuments(query, DocumentStatus::BANNED)[0].id == 54 && server.FindTopDocuments(query, DocumentStatus::BANNED).size() == 1);
	ASSERT(server.FindTopDocuments(query, DocumentStatus::REMOVED)[0].id == 78 && server.FindTopDocuments(query, DocumentStatus::REMOVED).size() == 1);
 //проверяем, что поиск правильно обрабатывает предикат пользователя
	ASSERT(server.FindTopDocuments(query, [](int document_id, DocumentStatus status, int rating) {
		return document_id == 54 && status == DocumentStatus::BANNED && rating == 3;
		})[0].id == 54 && server.FindTopDocuments(query, [](int document_id, DocumentStatus status, int rating) {
			return document_id == 54 && status == DocumentStatus::BANNED && rating == 3;
			}).size() == 1);

	ASSERT(server.FindTopDocuments(query, [](int document_id, DocumentStatus status, int rating) {
		return rating == 3;
		}).size() == 3);
}

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
	TestExcludeStopWordsFromAddedDocumentContent();
	TestMinusWords();
	TestMatchWords();
	TestRelevanceAndRating();
	TestStatusAndSelector();
	// Не забудьте вызывать остальные тесты здесь
}

// --------- Окончание модульных тестов поисковой системы -----------