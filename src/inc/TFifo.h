/*
 * TFifo.h
 *
 *  Created on: Sep 18, 2014
 *      Author: user
 */

#ifndef TFIFO_H_
#define TFIFO_H_

#include <deque>
#include <pthread.h>
#include <stdio.h>
#include <Log.h>
#include <fstream>

using namespace std;
using namespace log4net;

namespace MyClassTamlates
{

	template <class T>
	class TFifo
	{
		public:
			TFifo(int sz)
			{
				max_size = sz;
				siz = 0;
				int st = pthread_mutex_init(&sych, NULL);
				if(st != 0 ) Log::DEBUG("TFifo Error in pthread_mutex_init st="+toString(st));
			}
			virtual ~TFifo()
			{
			//pthread_mutex  mtx;
				int st = pthread_mutex_destroy(&sych);
				if(st != 0 ) Log::DEBUG("TFifo Error in pthread_mutex_destroy st="+toString(st));
			}

			void push(const T &val)
			{
				int st = pthread_mutex_lock(&sych);
				if(st != 0 ) Log::DEBUG("TFifo Error in pthread_mutex_lock st="+toString(st));
				if(siz >= max_size){
					dec.pop_front();
				}
				else{
					siz++;
				}
				dec.push_back(val);
				st = pthread_mutex_unlock(&sych);
				if(st != 0 ) Log::DEBUG("TFifo Error in pthread_mutex_unlock st="+toString(st));
			}

			T pop(void)
			{
				T ret;
				int st = pthread_mutex_lock(&sych);
				if(st != 0 ) Log::DEBUG("TFifo Error in pthread_mutex_lock st="+toString(st));
				if(dec.empty()){
					siz = 0;
				}
				else{
					ret = dec.front();
					dec.pop_front();
					siz--;
				}
				st = pthread_mutex_unlock(&sych);
				if(st != 0 ) Log::DEBUG("TFifo Error in pthread_mutex_unlock st="+toString(st));
				return ret;
			}


			T front(void)
			{
				T ret;
				int st = pthread_mutex_lock(&sych);
				if(st != 0 ) Log::DEBUG("TFifo Error in pthread_mutex_lock st="+toString(st));
				if(dec.empty()){
					siz = 0;
				}
				else{
					ret = dec.front();
				}
				st = pthread_mutex_unlock(&sych);
				if(st != 0 ) Log::DEBUG("TFifo Error in pthread_mutex_unlock st="+toString(st));
				return ret;
			}

			T back(void)
			{
				T ret;
				int st = pthread_mutex_lock(&sych);
				if(st != 0 ) Log::DEBUG("TFifo Error in pthread_mutex_lock st="+toString(st));
				if(dec.empty()){
					siz = 0;
				}
				else{
					ret = dec.back();
				}
				st = pthread_mutex_unlock(&sych);
				if(st != 0 ) Log::DEBUG("TFifo Error in pthread_mutex_unlock st="+toString(st));
				return ret;
			}


			int empty()
			{
				if(dec.empty()){
					siz = 0;
				}
				return siz == 0;
			}

			int size()
			{
				return siz;
			}

			void save(string filename)
			{
				try
				{
					FILE *fp;
					if((fp=fopen(filename.c_str(), "w"))==NULL)
					{
						Log::ERROR("Cannot open for save file="+filename);
					}
					else
					{
						//Log::DEBUG("pthread_mutex_lock");
						int st = pthread_mutex_lock(&sych);
						if(st != 0 ) Log::DEBUG("TFifo Error in pthread_mutex_lock st="+toString(st));
						for(int i = 0; i < dec.size(); i++)
						{
							T curr = dec[i];
							if(typeid(std::string) == typeid(curr)){
								string str = trimAllNull((string)curr)+"\n";
								if(!isNullOrWhiteSpace(str)){
									fputs(str.c_str(), fp);
								}
							}
						}
						st = pthread_mutex_unlock(&sych);
						if(st != 0 ) Log::DEBUG("TFifo Error in pthread_mutex_unlock st="+toString(st));

						fclose(fp);
						Log::DEBUG("TFifo save fifo_file="+filename);
					}
				}
				catch(exception &e)
				{
					Log::ERROR("[TFifo] save error: " + toString(e.what()) );
				}
			}

			void load(string filename)
			{
				try
				{
					FILE *fp;
					char cc[1024];
					if((fp=fopen(filename.c_str(), "r"))==NULL){
						Log::ERROR("Cannot open for load file="+filename);
					}
					else{
						while((fgets(cc, 1024, fp))!=NULL){
							string n = cc;
							if(!isNullOrWhiteSpace(n)){
								push(n);
								Log::DEBUG("TFifo push fifo string="+n);
							}
						}
						fclose(fp);
					}
				}
				catch(exception &e)
				{
					Log::ERROR("[TFifo] load error: " + toString(e.what()) );
				}
			}

			bool isexist(string filename){
				return std::ifstream(filename.c_str()) ? true : false;
			}

			bool remove_file(string filename)
			{
				bool ret = false;
				try
				{
					ret =  remove(filename.c_str()) == 0;
				}
				catch(exception &e)
				{
					Log::ERROR("[TFifo] save error: " + toString(e.what()) );
				}
				return ret;
			}


		private:
			deque<T> dec;
			int max_size;
			int siz;
			pthread_mutex_t sych;

	};

	template <class T>
	class sync_deque
	{
		public:
			sync_deque()
			{
				int st = pthread_mutex_init(&sych, NULL);
				if(st != 0 ) Log::DEBUG("sync_deque Error in pthread_mutex_init st="+toString(st));
			}
			virtual ~sync_deque()
			{
				int st = pthread_mutex_destroy(&sych);
				if(st != 0 ) Log::DEBUG("~sync_deque Error in pthread_mutex_destroy st="+toString(st));
			}

			T front()
			{
				T ret;
				int st = pthread_mutex_lock(&sych);
				if(st != 0 ) Log::DEBUG("sync_deque.front Error in pthread_mutex_lock st="+toString(st));
					ret = dec.front();
				st = pthread_mutex_unlock(&sych);
				if(st != 0 ) Log::DEBUG("sync_deque.front Error in pthread_mutex_unlock st="+toString(st));
				return ret;
			}

			T  back()
			{
				T ret;
				int st = pthread_mutex_lock(&sych);
				if(st != 0 ) Log::DEBUG("sync_deque.back Error in pthread_mutex_lock st="+toString(st));
					ret = dec.back();
				st = pthread_mutex_unlock(&sych);
				if(st != 0 ) Log::DEBUG("sync_deque.back Error in pthread_mutex_unlock st="+toString(st));
				return ret;
			}

			void pop_front(void)
			{
				int st = pthread_mutex_lock(&sych);
				if(st != 0 ) Log::DEBUG("sync_deque.pop_front Error in pthread_mutex_lock st="+toString(st));
					dec.pop_front();
				st = pthread_mutex_unlock(&sych);
				if(st != 0 ) Log::DEBUG("sync_deque.pop_front Error in pthread_mutex_unlock st="+toString(st));
			}

			void pop_back(void)
			{
				int st = pthread_mutex_lock(&sych);
				if(st != 0 ) Log::DEBUG("sync_deque.pop_back Error in pthread_mutex_lock st="+toString(st));
					dec.pop_back();
				st = pthread_mutex_unlock(&sych);
				if(st != 0 ) Log::DEBUG("sync_deque.pop_back Error in pthread_mutex_unlock st="+toString(st));
			}

			void push_front(const T &val)
			{
				int st = pthread_mutex_lock(&sych);
				if(st != 0 ) Log::DEBUG("sync_deque.push_front Error in pthread_mutex_lock st="+toString(st));
					dec.push_front(val);
				pthread_mutex_unlock(&sych);
				if(st != 0 ) Log::DEBUG("sync_deque.push_front Error in pthread_mutex_unlock st="+toString(st));
			}

			void push_back(const T &val)
			{
				int st = pthread_mutex_lock(&sych);
				if(st != 0 ) Log::DEBUG("sync_deque.push_back Error in pthread_mutex_lock st="+toString(st));
					dec.push_back(val);
				st = pthread_mutex_unlock(&sych);
				if(st != 0 ) Log::DEBUG("sync_deque.push_back Error in pthread_mutex_unlock st="+toString(st));
			}

			inline bool empty()
			{
				bool ret;
				int st = pthread_mutex_lock(&sych);
				if(st != 0 ) Log::DEBUG("sync_deque.empty Error in pthread_mutex_lock st="+toString(st));
					ret = dec.empty();
				st = pthread_mutex_unlock(&sych);
				if(st != 0 ) Log::DEBUG("sync_deque.empty Error in pthread_mutex_unlock st="+toString(st));
				return ret;
			}

			inline int size()
			{
				int ret;
				int st = pthread_mutex_lock(&sych);
				if(st != 0 ) Log::DEBUG("sync_deque.size Error in pthread_mutex_lock st="+toString(st));
					ret = dec.size();
				st = pthread_mutex_unlock(&sych);
				if(st != 0 ) Log::DEBUG("sync_deque.size Error in pthread_mutex_unlock st="+toString(st));
				return ret;
			}

			void clear()
			{
				int st = pthread_mutex_lock(&sych);
				if(st != 0 ) Log::DEBUG("sync_deque.clear Error in pthread_mutex_lock st="+toString(st));
					dec.clear();
				st = pthread_mutex_unlock(&sych);
				if(st != 0 ) Log::DEBUG("sync_deque.clear Error in pthread_mutex_unlock st="+toString(st));

			}

			inline T operator[](int ix)
			{
				return getByIndex(ix);
			}


		private:
			deque<T> dec;
			pthread_mutex_t sych;

			inline T getByIndex(int ix)
			{
				T ret;
				int st = pthread_mutex_lock(&sych);
				if(st != 0 ) Log::DEBUG("sync_deque.getByIndex Error in pthread_mutex_lock st="+toString(st));
					ret = dec[ix];
				st = pthread_mutex_unlock(&sych);
				if(st != 0 ) Log::DEBUG("sync_deque.getByIndex Error in pthread_mutex_lock st="+toString(st));
				return ret;
			}

	};

	class sync_word
	{
	public:
		sync_word()
		{
			data = 0;
			pthread_mutex_init(&sych, NULL);
		}
		virtual ~sync_word()
		{
			pthread_mutex_destroy(&sych);
		}

		Word GetData()
		{
			Word ret;
			pthread_mutex_lock(&sych);
				ret = data;
			pthread_mutex_unlock(&sych);
			return ret;
		}

		void SetData(Word newData)
		{
			pthread_mutex_lock(&sych);
				data = newData;
			pthread_mutex_unlock(&sych);
		}

	private:

		Word 			data;
		pthread_mutex_t sych;
	};

	class sync_float
	{
	public:
		sync_float()
		{
			data = 0;
			pthread_mutex_init(&sych, NULL);
		}
		virtual ~sync_float()
		{
			pthread_mutex_destroy(&sych);
		}

		float GetData()
		{
			float ret;
			pthread_mutex_lock(&sych);
				ret = data;
			pthread_mutex_unlock(&sych);
			return ret;
		}

		void SetData(float newData)
		{
			pthread_mutex_lock(&sych);
				data = newData;
			pthread_mutex_unlock(&sych);
		}

	private:

		float 			data;
		pthread_mutex_t sych;
	};

	template <class elemType>
	class sync_type
	{
	public:
		sync_type()
		{
			//data = 0;
			pthread_mutex_init(&sych, NULL);
		}
		virtual ~sync_type()
		{
			pthread_mutex_destroy(&sych);
		}

		elemType GetData()
		{
			elemType ret;
			pthread_mutex_lock(&sych);
				ret = data;
			pthread_mutex_unlock(&sych);
			return ret;
		}

		void SetData(elemType newData)
		{
			pthread_mutex_lock(&sych);
				data = newData;
			pthread_mutex_unlock(&sych);
		}

/*
		elemType operator=( const elemType& right)
		{
			SetData( right );
			return GetData();
		}*/

	private:

		elemType 		data;
		pthread_mutex_t sych;
	};

}/* namespace  */
#endif /* TFIFO_H_ */
