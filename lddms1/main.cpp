#include "MotifWorker.h"
#include <ctime>
#include <cmath>
#include <getopt.h>
#include "utils.h"
#include <cstdio>

#ifndef NOMPI

#include <mpi.h>

#endif

#define MAX_MOTIFS_PER_PROC 100000

extern "C"
{
int checkMotif(const char *inputFile, const char *candidateFile, const char *outputFile, int l, int d2);
}


void *runWork(void *w) {
    ((MotifWorker *) w)->doWork();
    return NULL;
}

#ifndef NOMPI

void createWorkerThread(pthread_t &t, MotifWorker *w) {
    pthread_create(&t, NULL, runWork, (void *) w);
}

#endif

void force_quit() {
#ifndef NOMPI
    MPI::COMM_WORLD.Abort(1);
    MPI::Finalize();
#endif
    exit(1);
}

void printHelp() {
    cout
            << "Arguments: inputFile -l <motifLength> -D <Max#Mutations> -d <Min#Mutations> -c <candidateOutFile> [-o <outputFile>] [-h]"
            << endl;
    cout << "Where:" << endl;
    cout << "  -l: length of motif" << endl;
    cout << "  -D: max changes for planted instances of the motif" << endl;
    cout << "  -d: min changes for planted instances of the motif" << endl;
    cout << "  -c: candidateOutFile" << endl;
    cout << "  -o: outputFile [stdout if not specified]" << endl;
    cout << "  -h: print this help" << endl;
}

void parseArgs(int argc, char **argv, MotifConfig &mc, int *pointerl, int *pointerd2, string &inputFile, string &candidateOutFile,
               string &outputFile) {
    mc.L = -1;
    mc.d = -1;
    mc.minStackSize = -1;
    mc.nPrime = -1;
    mc.q_percent = 100;

    for (int c; (c = getopt(argc, argv, "o:l:D:d:c:h")) != -1;) {
        switch (c) {
            case 'h':
                printHelp();
                force_quit();
                break;
            case 'c':
                candidateOutFile = string(optarg);
                break;
            case 'o':
                outputFile = string(optarg);
                break;
            case 'l':
                parseIntOrExit(c, optarg, &mc.L);
                parseIntOrExit(c, optarg, pointerl);
                break;
            case 'D':
                parseIntOrExit(c, optarg, &mc.d);
                break;
            case 'd':
                parseIntOrExit(c, optarg, pointerd2);
                break;
            case '?':
                cerr << "Unknown option `-" << optopt
                     << "' or missing required argument." << endl;
                force_quit();
                break;
            default:
                force_quit();
        }
    }

    if (optind < argc)
        inputFile = string(argv[optind]);
    else {
        cerr << "No input file specified!" << endl;
        force_quit();
    }
}

int main(int argc, char **argv) {
    clock_t startTime = clock();
    int myRank = 0;
    int nProcs = 1;
    int totalMsgWords;
    int *b = NULL;

#ifndef NOMPI
    int MotifTransferTag = 2;
    MPI::Status status;
    MPI::Init_thread(argc, argv, MPI::THREAD_FUNNELED);
    myRank = MPI::COMM_WORLD.Get_rank();
    nProcs = MPI::COMM_WORLD.Get_size();
#endif

    if (argc == 1) {
        if (myRank == 0) {
            printHelp();
        }
        exit(0);
    }

    int l = 11;
    int *pointerl = &l;
    int d2 = 0;
    int *pointerd2 = &d2;
    MotifConfig mc;
    string inputFile;
    string candidateOutFile;
    string outputFile;

    if (myRank == 0) {

        
        parseArgs(argc, argv, mc, pointerl, pointerd2, inputFile, candidateOutFile, outputFile);

        b = MotifWorker::readAndEncodeInput(inputFile.c_str(), totalMsgWords, mc);
        if (b == NULL) {
            cerr << "Unable to open file " << inputFile << endl;
            force_quit();
        }
    }

#ifndef NOMPI
    MPI::COMM_WORLD.Bcast(&totalMsgWords, sizeof(int), MPI::CHAR, 0);
    if (myRank != 0) {
        b = new int[totalMsgWords];
    }

    MPI::COMM_WORLD.Bcast(b, totalMsgWords * sizeof(int), MPI::CHAR, 0);
#endif

    MotifWorker worker(myRank, nProcs, startTime, b);
    delete[] b;

//     //#ifndef NOMPI
//     //	if (myRank != 0) {
//     //		cerr << "Processor " << myRank << " received ";
//     //		worker.printConfig();
//     //	}
//     //#endif

    if (myRank == 0 && nProcs > 1) {
#ifndef NOMPI // should have MPI but we avoid pthread also if possible
        pthread_t thread;
        createWorkerThread(thread, &worker);
        worker.schedulerLoop();
        pthread_join(thread, NULL);
        cerr << endl;
#else
        cerr << "This should never happen" << endl;
        force_quit();
#endif
    } else {
        worker.doWork();
    }

    set<MyString> motifs = worker.getMotifs();

    if (myRank == 0) {
#ifndef NOMPI
        int sz;
        int *buf = worker.allocateMotifBuffer(MAX_MOTIFS_PER_PROC, sz);
        for (int i = 1; i < nProcs; ++i) {
            MPI::COMM_WORLD.Recv(buf, sz, MPI::INT, MPI::ANY_SOURCE,
                                 MotifTransferTag, status);
            worker.decodeMotifs(buf, motifs);
            //			int nm = worker.decodeMotifs(buf, motifs);
            //			cerr << "Processor " << myRank << " received " << nm
            //					<< " motifs from " << status.Get_source() << endl;
        }
        delete[] buf;
#endif

        if (!candidateOutFile.empty())
            freopen((const char *) candidateOutFile.c_str(), "w", (FILE *) stdout);
        worker.printMotifs(motifs);
        if (!candidateOutFile.empty())
            fclose((FILE *) stdout);
        cerr << "Total motifs found: " << motifs.size() << endl;
    } else {
#ifndef NOMPI
        int nMotifs = motifs.size();
        if (nMotifs > MAX_MOTIFS_PER_PROC) {
            cerr << "Processor " << myRank << " found " << motifs.size()
                 << " motifs; Keeping first " << MAX_MOTIFS_PER_PROC
                 << " of them" << endl;
            nMotifs = MAX_MOTIFS_PER_PROC;
        }
        int requiredMem;
        int *buf = worker.encodeMotifs(motifs, nMotifs, requiredMem);
        MPI::COMM_WORLD.Send(buf, requiredMem, MPI::INT, 0, MotifTransferTag);
        //		cerr << "Processor " << myRank << " sent " << nMotifs
        //				<< " motifs to proc 0" << endl;
        delete[] buf;
#endif
    }

    const char *inputFileConverted = inputFile.c_str();
    const char *candidateOutFileConverted = candidateOutFile.c_str();
    const char *outputFileConverted = outputFile.c_str();

    int i = checkMotif(inputFileConverted, candidateOutFileConverted, outputFileConverted, l, d2);
    cerr << "Number of motifs found is " << i << '\n';
    if (myRank == 0) {
        clock_t endTime = clock();
        float seconds = (float) (endTime - startTime) / CLOCKS_PER_SEC;
        cerr << "Time on processor: " << seconds << " s" << endl;
    }

#ifndef NOMPI
    MPI::Finalize();
#endif
    return 0;
}

