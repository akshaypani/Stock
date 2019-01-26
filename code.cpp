#include <iterator>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <queue>
#include <windows.h>
#include <conio.h>
#include <stdlib.h>
//#include <chrono>
//#include <thread>

using namespace std;

float round(double var) 
{ 
    int value = (int)(var * 1000 + .5); 
    return (float)value / 1000; 
} 

class CSVRow
{
    public:
        string const& operator[](size_t index) const
        {
            return m_data[index];
        }
        size_t size() const
        {
            return m_data.size();
        }
        void readNextRow(istream& str)
        {
            string         line;
            getline(str, line);

            stringstream   lineStream(line);
            string         cell;

            m_data.clear();
            while(getline(lineStream, cell, ','))
            {
                m_data.push_back(cell);
            }
            // This checks for a trailing comma with no data after it.
            if (!lineStream && cell.empty())
            {
                // If there was a trailing comma then add an empty element.
//                m_data.push_back("");
            }
        }
        void readLastRow(istream& fin)
        {
        	fin.seekg(-3,ios_base::end);                // go to one spot before the EOF

        	bool keepLooping = true;
	        while(keepLooping) {
	            char ch;
	            fin.get(ch);                            // Get current byte's data
			
	            if((int)fin.tellg() <= 1) {             // If the data was at or before the 0th byte
	                fin.seekg(0);                       // The first line is the last line
	                keepLooping = false;                // So stop there
	            }
	            else if(ch == '\n') {                   // If the data was a newline
	                keepLooping = false;                // Stop at the current position.
	            }
	            else {                                  // If the data was neither a newline nor at the 0 byte
	                fin.seekg(-3,ios_base::cur);        // Move to the front of that data, then to the front of the data before it
	            }
	        }

	        readNextRow(fin);
		}
		void display(void)
		{
			for(int i=0; i<m_data.size(); ++i)
				cout<<m_data[i]<<"\t\t";
		}
		void push_back(string s)
		{
			m_data.push_back(s);
		}
    private:
        vector<string>    m_data;
};

istream& operator>>(istream& str, CSVRow& data)
{
    data.readNextRow(str);
    return str;
}


class Trade
{
	private:
		CSVRow	row;
		string	timestamp;
		double	close, open, high, low, t_price; 
		double sum_c_price, EMA_12, EMA_26, MACD, signal_macd, sum_macd, histogram_macd;
		double up_move, down_move, sum_up_move, sum_down_move, avg_up_move, avg_down_move, RS, RSI;
		double SMA;
		unsigned int vol, cumul_vol, m_cumul_vol, bid_vol;
		double	VWAP, MVWAP;
		double profit, loss;
		double entry, stop, target;
		bool trade_type;
		bool trend, trend_sma;
		string path;
		deque<pair<string,vector<double> > > mov_history;
		vector<double> data, init_data;
		
		void calcIndicators(void);
		
		void displayIndicators(void)
		{
//			for(int i=13; i<data.size(); ++i)
//				cout<<round(data[i])<<"\t\t";
			cout<<round(data[6])<<"\t\t"<<round(data[7])<<"\t\t"<<round(data[10])<<"\t\t"<<round(data[18])<<"\t\t"<<round(data[19])<<"\t\t";
		}
		
		void displayTitle(void)
		{
			cout<<"Date\t\t\tClose\t\tVolume\t\t\tOpen\t\tHigh\t\tLow\t\t\tVWAP\t\tMVWAP\t\tMACD\t\tRSI\t\tSMA\t\tVWAP\t\tMVWAP\t\tMACD\t\tRSI\t\tSMA\n";
		}
		
		string vwapStrategy(void);
		string macdStrategy(void);
		string rsiStrategy(void);
		string mvwapStrategy(void);
		string smaStrategy(void);
		
		void displayDecisions(void)
		{
			cout<<vwapStrategy()<<"\t\t"<<mvwapStrategy()<<"\t\t"<<macdStrategy()<<"\t\t"<<rsiStrategy()<<"\t\t"<<smaStrategy();
			cout<<"\n";
		}
		
	public:
		void Start(bool,string);
		void Stop(void);
		
	

};

void Trade::calcIndicators()
{
	close = atof(row[1].c_str());
	open = atof(row[3].c_str());
	high = atof(row[4].c_str());
	low = atof(row[5].c_str());
	vol = atof(row[2].c_str());
	
	double a[] = {vol,close,open,high,low};
	data.clear();
	data.insert(data.begin(),&a[0],&a[5]);
	
	t_price = (high+low+close)/3;
	data.push_back(t_price);
	
	VWAP = (cumul_vol*VWAP + vol*t_price)/(cumul_vol+vol);
	data.push_back(VWAP);
	cumul_vol += vol;
	
	
	
	if(mov_history.size()>=5)
	{
		init_data.clear();
		init_data = mov_history.at(4).second;
		MVWAP = (m_cumul_vol*MVWAP - init_data[0]*init_data[5]+ vol*t_price)/(m_cumul_vol+vol-init_data[0]);
		m_cumul_vol += vol - init_data[0];
	}
	else
	{
		MVWAP = VWAP;
		m_cumul_vol = cumul_vol;	
	}
	data.push_back(MVWAP);
	
	
	
	sum_c_price+=close;
	
	if(mov_history.size()<12)
		EMA_12 = sum_c_price/(mov_history.size()+1);
	else
		EMA_12 = 2.0/13.0*close+mov_history.at(0).second[8]*11.0/13.0;	
	data.push_back(EMA_12);
	
	
	
	if(mov_history.size()<26)
		EMA_26 = sum_c_price/(mov_history.size()+1);
	else
		EMA_26 = 2.0/27.0*close+mov_history.at(0).second[9]*25.0/27.0;	
	data.push_back(EMA_26);
	
	
	
	if(mov_history.size()<25)
		MACD = 0;
	else
		MACD = EMA_12-EMA_26;	
	data.push_back(MACD);
	
	
	
	sum_macd += MACD;
	histogram_macd = 0;
	if(mov_history.size()==33)
		signal_macd = sum_macd/9;
	else if(mov_history.size()>33)
	{
		signal_macd = 0.2*MACD+mov_history.at(0).second[11]*0.8;
		histogram_macd = MACD - signal_macd;
	}
	else
		signal_macd = 0;
	data.push_back(signal_macd);
	data.push_back(histogram_macd);
	
	
	up_move = down_move = 0;
	if(mov_history.size()>=1)
	{
		if(close>mov_history.at(0).second[1])
			up_move = close-mov_history.at(0).second[1];
		else
			down_move = mov_history.at(0).second[1]-close;
	}	
	data.push_back(up_move);
	data.push_back(down_move);
	
	
	if(mov_history.size()<13)
	{
		sum_up_move += up_move;
		sum_down_move += down_move;
	}
	else if(mov_history.size()==13)
	{
		avg_up_move = sum_up_move/14;
		avg_down_move = sum_down_move/14;
		RS = avg_up_move/avg_down_move;
		RSI = 100*RS/(RS+1);
	}
	else
	{
		avg_up_move = (up_move + mov_history.at(0).second[15]*13)/14.0;
		avg_down_move = (down_move + mov_history.at(0).second[16]*13)/14.0;
		RS = avg_up_move/avg_down_move;
		RSI = 100*RS/(RS+1);
	}	
	data.push_back(avg_up_move);
	data.push_back(avg_down_move);
	data.push_back(RS);
	data.push_back(RSI);
	
	
	if(mov_history.size()>=14)
	{
		init_data.clear();
		init_data = mov_history.at(13).second;
		SMA = (mov_history.at(0).second[19]*14 - init_data[1]+ close)/14;
	}
	else
	{
		SMA = sum_c_price/(mov_history.size()+1);	
	}
	data.push_back(SMA);
	
}

string Trade::vwapStrategy(void)
{
	string decision = "neutral";
	
	if(mov_history.size()>=1)
	{
	if(close>=mov_history.at(0).second[1])
		trend = 1;
	else
		trend = 0;
	
	if(close>VWAP&&mov_history.at(0).second[1]<VWAP)
		decision="Buy(L)";	
	else if(close<VWAP&&mov_history.at(0).second[1]>VWAP)
		decision="Sell(S)";
	else if(close>VWAP&&trend==0)
		decision="Sell(L)";
	else if(close<VWAP&&trend==1)
		decision="Buy(S)";
	}
	
	return decision;
}

string Trade::mvwapStrategy(void)
{
	string decision = "neutral";
	
	if(mov_history.size()>=1)
	{
	if(close>=mov_history.at(0).second[1])
		trend = 1;
	else
		trend = 0;
	
	if(close>MVWAP&&mov_history.at(0).second[1]<MVWAP)
		decision="Buy(L)";	
	else if(close<MVWAP&&mov_history.at(0).second[1]>MVWAP)
		decision="Sell(S)";
	else if(close>MVWAP&&trend==0)
		decision="Sell(L)";
	else if(close<MVWAP&&trend==1)
		decision="Buy(S)";
	}
	
	return decision;
}


string Trade::macdStrategy(void)
{
	string decision = "neutral";
	if(mov_history.size()>=35)
	{
	if(histogram_macd>0&&mov_history.at(0).second[12]<0)
		decision="Buy";	
	else if(histogram_macd<0&&mov_history.at(0).second[12]>0)
		decision="Sell";
	}
	
	return decision;
}

string Trade::rsiStrategy(void)
{
	string decision = "neutral";
	if(mov_history.size()>=14)
	{
	if(RSI<30&&mov_history.at(0).second[18]>30)
		decision="Buy";	
	else if(RSI>70&&mov_history.at(0).second[18]<70)
		decision="Sell";
	}
	
	return decision;
}

string Trade::smaStrategy(void)
{
	string decision = "neutral";
	if(mov_history.size()>=14)
	{
	if(SMA>mov_history.at(5).second[19])
		decision="Buy";	
	else if(SMA<mov_history.at(5).second[19])
		decision="Sell";
	}
	
	return decision;
}


void Trade::Start(bool trade_type_in,string path_in)
{
	trade_type = trade_type_in;
	path = path_in;
	cumul_vol = 0;
	m_cumul_vol = 0;
	VWAP = 0;
	MVWAP = 0;
	
	sum_c_price = 0;
	sum_macd = 0;
	
	sum_up_move = avg_up_move = 0;
	sum_down_move = avg_down_move = 0;
	
		
	ifstream	file(&path[0]);
	
	displayTitle();
	
	while(file >> row)
    {
        calcIndicators();
		row.display();
		displayIndicators();
		displayDecisions();
		
		if(mov_history.size()<35)
			mov_history.push_front(make_pair(row[0],data));
		else
		{
			mov_history.push_front(make_pair(row[0],data));
			mov_history.pop_back();
		}
    }
    file.close();
    while(1)
    {
        cout<<"Searching"<<"\n";
		Sleep(10000);
		ifstream	file(&path[0]);
		row.readLastRow(file);
		
		if(mov_history.front().first==row[0])
			break;
			
		calcIndicators();
		row.display();
		displayIndicators();
		displayDecisions();
		
		mov_history.push_front(make_pair(row[0],data));
			
		if(mov_history.size()>35)
			mov_history.pop_back();
		
    }
    
    file.close();

    cout<<"Ended";
    
}




int main()
{
//    clrscr();
	Trade T;
    T.Start(0,"HistoricalQuotes.csv");
    getch();
    return 0;
}

