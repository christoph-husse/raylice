// ======================================================================== //
// Copyright 2013 Christoph Husse                                           //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //


template<class TValue>
class ProgressBar : boost::noncopyable
{
private:
	std::atomic<TValue> current;
	const TValue maximum;
	std::atomic<int> prevProgress;
	std::mutex mutex;

	void Update()
	{
		int progress = (int)(current / (float)maximum * 100);

		if(progress > prevProgress)
		{
			std::lock_guard<std::mutex> lock(mutex);

			progress = (int)(current / (float)maximum * 100);

			for(; prevProgress < progress;)
			{
				prevProgress++;
				if(prevProgress % 10)
					std::cout << ".";	
				else
					std::cout << prevProgress << "%";	
			}
		}
	}
public:

	ProgressBar(TValue maximum) : current(), maximum(maximum), prevProgress(0)
	{
	}

	ProgressBar& operator =(TValue delta)
	{
		current = delta;
		Update();
		return *this;
	}

	ProgressBar& operator +=(TValue delta)
	{
		current += delta;
		Update();
		return *this;
	}
};

