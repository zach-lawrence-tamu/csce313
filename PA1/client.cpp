/*
   Author of the starter code
   Yifan Ren
   Department of Computer Science & Engineering
   Texas A&M University
Date: 9/15/2024

Please include your Name, UIN, and the date below
Name: Zach Lawrence
UIN: 132008791
Date: 9/25/2024
 */
#include "common.h"
#include "FIFORequestChannel.h"

using namespace std;

int main (int argc, char *argv[]) {
		int opt;
		int p = -1;
		//for testing, I set these to -1, reset t = 0.0, e = 1
		double t = -1;
		int e = -1;
		int msgSize = MAX_MESSAGE;
		string filename = "";
		bool new_channel = false;
		vector<FIFORequestChannel*> channels;

		//Add other arguments here
		while ((opt = getopt(argc, argv, "p:t:e:f:m:c")) != -1) {
				switch (opt) {
						case 'p':
								p = atoi (optarg);
								break;
						case 't':
								t = atof (optarg);
								break;
						case 'e':
								e = atoi (optarg);
								break;
						case 'f':
								filename = optarg;
								break;
						case 'm':
								msgSize = atoi(optarg);
								break;
						case 'c':
								new_channel = true;
								break;
				}
		}

		/*
		   basically use if and else statements and paramaterize everything instead of hardcode
		   also you can use diff file1 file2 to get the difference between files (good for debug)
		 */

		//Task 1:
		//Run the server process as a child of the client process
		pid_t pid = fork();
		if (pid < 0)
		{
				return -1;
		}
		else if (pid == 0)
		{
				//child process, make server here using execvp
				//server needs ./server, -m, val for -m args, and nullptr
				
				char* serverArgs[] = {(char*) "./server", (char*) "-m", (char*) to_string(msgSize).c_str(), nullptr};
				execvp("./server", serverArgs);
		}
		else
		{
				//client side pipe named control
				FIFORequestChannel control_chan("control", FIFORequestChannel::CLIENT_SIDE);
				channels.push_back(&control_chan);

				//Task 4:
				//Request a new channel
				if (new_channel)
				{
					//send new channel request to server
					MESSAGE_TYPE new_chan = NEWCHANNEL_MSG;
					control_chan.cwrite(&new_chan, sizeof(MESSAGE_TYPE));
					//recieve new name of channel from server
					char* c_name = new char[30];
					control_chan.cread(c_name, sizeof(c_name));
					//FIFOrequestChannel constructor with new channel name
					FIFORequestChannel* chan1 = new FIFORequestChannel(c_name, FIFORequestChannel::CLIENT_SIDE);
					channels.push_back(chan1);
					//c_name[29] = '\0';
					//cout << "c: " << c_name << endl;
					delete[] c_name;
				}

				FIFORequestChannel chan = *(channels.back());

				//Task 2:
				//Request data points
				if (p != -1 && t != -1 && e != -1)
				{
					//request only 1 data point
					char buf[MAX_MESSAGE];
					datamsg x(p, t, e);

					memcpy(buf, &x, sizeof(datamsg));
					chan.cwrite(buf, sizeof(datamsg));
					double reply;
					chan.cread(&reply, sizeof(double));
					cout << "For person " << p << ", at time " << 
					t << ", the value of ecg " << e << " is " << reply << endl;
				}
				else if (p != -1)
				{
					//segfaults if t = -1 and e = -1
					//request 1000 data points
					//loop over first 1000 lines in sreadsheet
					//can use whatever method to open and read the file
					//like fopen, etc.
					//send request for ecg 1
					//send request for ecg 2
					//write line to recieved/x1.csv
					ofstream csvOut("received/x1.csv");
					if (csvOut.is_open())
					{
						t = 0;
						for (int i = 0; i < 1000; i++)
						{
							char buf[MAX_MESSAGE];
							//ecg1 = 1, ecg2 = 2
							e = 1;
							datamsg x(p, t, e);
							csvOut << t << ',';
							t += .004;
							
							//ecg 1 from server
							memcpy(buf, &x, sizeof(datamsg));
							chan.cwrite(buf, sizeof(datamsg));
							double reply;
							chan.cread(&reply, sizeof(double));
							csvOut << reply << ',';
							
							//cout << "For person " << p << ", at time " << 
							//t << ", the value of ecg " << e << " is " << reply << endl;
							
							//ecg 2 from server
							x.ecgno = 2;
							memcpy(buf, &x, sizeof(datamsg));
							chan.cwrite(buf, sizeof(datamsg));
							chan.cread(&reply, sizeof(double));
							csvOut << reply << endl;
							
							//cout << "For person " << p << ", at time " << 
							//t << ", the value of ecg " << e << " is " << reply << endl;
						}
					}
					else
					{
						cerr << "Error opening file" << endl;
					}
					csvOut.close();
				}

				//Task 3:
				//Request files
				if (filename != "")
				{
					filemsg fm(0, 0);

					int len = sizeof(filemsg) + (filename.size() + 1);
					char* buf2 = new char[len];
					memcpy(buf2, &fm, sizeof(filemsg));
					strcpy(buf2 + sizeof(filemsg), filename.c_str());
					chan.cwrite(buf2, len);
					
					__int64_t file_length;
					chan.cread(&file_length, sizeof(__int64_t));
					cout << "The length of " << filename << " is " << file_length << endl;
					
					ofstream fileOut("received/" + filename, std::ios_base::binary);
					
					//make response buffer
					char* respBuf = new char[msgSize];
					
					//cout << "received/" << filename << endl;
					//cout << msgSize << endl;
					
					if (fileOut.is_open())
					{
						//we now know the file length and need to parse the data from the file on the server
						//loop over segments in file (filesize / buff capacity(msgSize))
						for (int i = 0; i <= file_length/msgSize; i++)
						{
							filemsg* fileReq = (filemsg*)buf2;
							fileReq->offset = msgSize*i;//set offset in file
							fileReq->length = msgSize;//set length, be careful of last segment as it 
							//may be bigger than remainder of file left
							if (msgSize >= file_length - fileReq->offset)
							{
								fileReq->length = file_length - fileReq->offset;
							}

							//std::cout << fileReq->offset << " " << fileReq->length << endl;
							
							//send request (buf2) like earlier
							chan.cwrite(buf2, len);
							//receive response buffer using cread with length fileReq->length
							chan.cread(respBuf, fileReq->length);
							//write respBuf to file received/filename
							
							//respBuf[msgSize-1] = '\0';
							//std::cout << respBuf << std::endl;
							//std::cout << sizeof(respBuf) << std::endl;
							//fileOut << respBuf;
							
							//if you use << to write to file using ostream, I think it writes until null terminator
							fileOut.write(respBuf, fileReq->length);
						}
					}
					else
					{
						cerr << "Error opening file" << endl;
					}
					fileOut.close();
					delete[] buf2;
					delete[] respBuf;
				}


				//Task 5:
				// Closing all the channels
				MESSAGE_TYPE m = QUIT_MSG;
				
				if (new_channel)
				{
					//close and delete channel
					chan.cwrite(&m, sizeof(MESSAGE_TYPE));
					FIFORequestChannel* temp = channels.back();
					channels.pop_back();
					delete temp;
				}
				
				chan = *(channels.back());
				m = QUIT_MSG;
				chan.cwrite(&m, sizeof(MESSAGE_TYPE));
				channels.pop_back();
				//int status;
				//wait(&status);
		}
}
