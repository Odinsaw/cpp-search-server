
#include <iostream>
#include "read_input_functions.h"
#include "string_processing.h"
#include "document.h"
#include "search_server.h"
#include "paginator.h"
#include "request_queue.h"
#include "log_duration.h"
#include "remove_duplicates.h"

using namespace std;

int main() {
	system("chcp 1251");
	//SearchServer search_server("and in at"s);

	//RequestQueue request_queue(search_server);

	//search_server.AddDocument(1, "curly cat curly tail"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
	//search_server.AddDocument(2, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
	//search_server.AddDocument(3, "big cat fancy collar "s, DocumentStatus::ACTUAL, { 1, 2, 8 });
	//search_server.AddDocument(4, "big dog sparrow Eugene"s, DocumentStatus::ACTUAL, { 1, 3, 2 });
	//search_server.AddDocument(5, "big dog sparrow Vasiliy"s, DocumentStatus::ACTUAL, { 1, 1, 1 });



	//// 1439 �������� � ������� �����������
	//for (int i = 0; i < 1439; ++i) {
	//	request_queue.AddFindRequest("empty request"s);
	//}
	//// ��� ��� 1439 �������� � ������� �����������
	//request_queue.AddFindRequest("curly dog"s, DocumentStatus::BANNED);
	//// ����� �����, ������ ������ ������, 1438 �������� � ������� �����������
	//request_queue.AddFindRequest("big collar"s, [](int document_id, DocumentStatus status, int rating) {return document_id == 4; });
	//// ������ ������ ������, 1437 �������� � ������� �����������
	//request_queue.AddFindRequest("sparrow"s);
	//cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;

	//SearchServer search_server("� � ��"s);

	//search_server.AddDocument(1, "�������� ��� �������� �����"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
	//search_server.AddDocument(4, "������� �� ������� �������"s, DocumentStatus::ACTUAL, { 1, 1, 1 });
	//{
	//	LOG_DURATION("1"s);
	//	for (auto doc : search_server.FindTopDocuments("�������� ��"s)) {
	//		PrintDocument(doc);
	//	}

	//}
	//search_server.MatchDocument("�������� ��"s, DocumentStatus::ACTUAL);
	//search_server.MatchDocument("������ -���"s, DocumentStatus::ACTUAL);

		SearchServer search_server("and with"s);

		search_server.AddDocument(1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
		search_server.AddDocument( 2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

		// �������� ��������� 2, ����� �����
		search_server.AddDocument( 3, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

		// ������� ������ � ����-������, ������� ����������
		search_server.AddDocument( 4, "funny pet and curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

		// ��������� ���� ����� ��, ������� ���������� ��������� 1
		search_server.AddDocument( 5, "funny funny pet and nasty nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });

		// ���������� ����� �����, ���������� �� ��������
		search_server.AddDocument( 6, "funny pet and not very nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });

		// ��������� ���� ����� ��, ��� � id 6, �������� �� ������ �������, ������� ����������
		search_server.AddDocument(7, "very nasty rat and not very funny pet"s, DocumentStatus::ACTUAL, { 1, 2 });

		// ���� �� ��� �����, �� �������� ����������
		search_server.AddDocument( 8, "pet with rat and rat and rat"s, DocumentStatus::ACTUAL, { 1, 2 });

		// ����� �� ������ ����������, �� �������� ����������
		search_server.AddDocument( 9, "nasty rat with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

			//for (int i = 0; i < 500; ++i) {
			//	//search_server.AddDocument(i, "nasty rat with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });
			//	search_server.AddDocument(i, to_string(i), DocumentStatus::ACTUAL, { 1, 2 });
			//}
			//for (int i = 500; i < 750; ++i) {
			//	//search_server.AddDocument(i, "nasty rat with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });
			//	search_server.AddDocument(i,"pet with rat and rat and rat"s, DocumentStatus::ACTUAL, { 1, 2 });
			//}
			//for (int i = 750; i < 1000; ++i) {
			//	//search_server.AddDocument(i, "nasty rat with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });
			//	search_server.AddDocument(i, "pet with rat and rat and rat"s, DocumentStatus::ACTUAL, { 1, 2 });
			//}

		cout << "Before duplicates removed: "s << search_server.GetDocumentCount() << endl;
		{
			LOG_DURATION("Duplicate remove"s);
			RemoveDuplicates(search_server);
		}
		cout << "After duplicates removed: "s << search_server.GetDocumentCount() << endl;


	return 0;
}
