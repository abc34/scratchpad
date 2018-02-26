(function()
{
  //console.time("Elapsed (tree fast)");
  var N = 32,/*длина блока*/
      m,     /*количество различных длин*/
      n_arr, /*массив длин, упорядоченные по возрастанию и не превышающие N*/
      v_arr, /*массив распределений длин*/
      s_arr, /*массив частичных сумм*/
      d_arr, /*приращения длин*/
      q_arr, /*массив числа комбинаций в зависимости от частичных сумм*/
      c = 0; /*счётчик*/
  n_arr = (new Uint16Array(8)).map((v,i)=>{return i+1;});
  //n_arr = new Uint16Array([1,3,5,8]);/*зададим длины*/
  //n_arr = new Uint16Array([1,2,3,5,8,13,21,29]);/*зададим длины*/
  v_arr = new Uint8Array(N/n_arr[0]+1);
  s_arr = new Uint16Array(v_arr.length);
  n_arr.sort(); /*force sort by asc order*/
  m = n_arr.length;
  d_arr = (new Uint16Array(n_arr.length)).map((v,i)=>{return i<m-1?n_arr[i+1]-n_arr[i]:N;});
  q_arr = (new Array(N)).fill(0);
  /*Подсчёт всевозможных комбинаций длин в фиксированном блоке*/
  /*поиск по дереву fast*/
  var y,j=0,nmin=n_arr[0];v_arr[0]=0;s_arr[0]=N-nmin;
  while(1)
  {
    y=false;
    if(q_arr[s_arr[j]]==0)
    {
    	while(s_arr[j]>=nmin){j++;v_arr[j]=0;s_arr[j]=s_arr[j-1]-nmin;}
    }
    else
    {
      c+=q_arr[s_arr[j]];y=true;
    }
    while(s_arr[j]<d_arr[v_arr[j]])
    {
      if(s_arr[j]==0)
      {
        c++;y=true;
      }
      j--;if(j<0)break;
      if(y && q_arr[s_arr[j]]==0)
      {
        q_arr[s_arr[j]]=c;
      }
    }
    if(j<0)break;
    s_arr[j]-=d_arr[v_arr[j]];v_arr[j]++;
  }


  console.log("N = %d, total = %o", N, c);
  //console.log("n_arr = "+n_arr);
  //console.log("q_arr = "+q_arr);

  //console.timeEnd("Elapsed (tree fast)");
  //console.log("----------------------------");  
  /*N=32  m=8 c=2043730736  t=0.0016 sec*/
  /*N=32  m=8 c=431846836   n_arr=[1,2,3,5,8,13,21,29] t=0.002 sec*/
  /*N=40  m=8 c=71913200393 n_arr=[1,2,3,5,8,13,21,29] t=0.002 sec*/
  /*N=53  m=8 c=4110904791762176  t=0.002 sec*/
  /*N=54  m=8 c=8205494141281025  t=0.002 sec*/
  /*N>54 происходит потеря точности представления double при превышении 9e15*/
  /*     в firefox почему-то выводит все знаки (при %f), хотя возможно, что только 21 точные*/
}
)();







(function()
{
  console.time("Elapsed (tree)");
  var N = 32,/*длина блока*/
      m,     /*количество различных длин*/
      v_arr, /*массив распределений длин*/
      s_arr, /*массив частичных сумм*/
      n_arr, /*массив длин, упорядоченные по возрастанию и не превышающие N*/
      c_arr, /*массив счетчиков количества комбинаций в зависимости от длины*/
      d_arr; /*приращения длин*/
  n_arr = (new Uint16Array(8)).map((v,i)=>{return i+1;});
  //n_arr = new Uint16Array([1,3,5,8]);/*зададим длины*/
  //n_arr = new Uint16Array([1,2,3,5,8,13,21,29]);/*зададим длины*/
  v_arr = new Uint8Array(N/n_arr[0]+1);
  s_arr = new Uint8Array(v_arr.length);
  c_arr = (new Array(v_arr.length)).fill(0);
  n_arr.sort(); /*force sort by asc order*/
  m = n_arr.length;
  d_arr = (new Uint16Array(n_arr.length)).map((v,i)=>{return i<n_arr.length-1?n_arr[i+1]-n_arr[i]:N;});

  /*Распределение всевозможных комбинаций длин в фиксированном блоке*/
  /*поиск по дереву*/
  var j=0,nmin=n_arr[0];v_arr[0]=0;s_arr[0]=N-nmin;
  while(1)
  {
      /*подбор мин.длины*/
      while(s_arr[j]>=nmin){j++;v_arr[j]=0;s_arr[j]=s_arr[j-1]-nmin;}
      while(s_arr[j]<d_arr[v_arr[j]])
      {
        if(s_arr[j]==0)
        {
          //console.log("v="+v_arr.subarray(0,j+1).map((v)=>{return n_arr[v];})+"\ns="+s_arr.subarray(0,j+1));
          c_arr[j+1]++;
        }
        j--;if(j<0)break;
      }
      if(j<0)break;
      s_arr[j]-=d_arr[v_arr[j]];v_arr[j]++;
  }


  console.log("total = "+c_arr.reduce((s,v)=>{return s+v;}));
  console.log("c_arr = "+c_arr);
  console.log("n_arr = "+n_arr);
  /*N=16 m=4 c=20569*/
  /*N=16 m=8 c=32192*/
  /*N=16 m=8 c=24219 n_arr=[1,2,3,4,6,8,10,13]*/
  /*N=16 m=8 c=12744 n_arr=[1,2,3,6,12,13,14,15]*/
  /*N=16 m=8 c=16298 n_arr=[1,2,3,5,7,10,13,14]*/
  /*N=32 m=8 c=2043730736 t=17 sec*/
  /*N=32 m=8 c= 431846836 n_arr=[1,2,3,5,8,13,21,29] t=4 sec*/
  /*N    m=8 c< 1<<(N-1)*/

  console.timeEnd("Elapsed (tree)");
  console.log("----------------------------");  
/*test:  N=8  n_arr=[1,3,5,8]*/
//1                  2              3              4              5          6
//v=1,1,1,1,1,1,1,1  v=1,1,1,1,1,3  v=1,1,1,1,3,1  v=1,1,1,3,1,1  v=1,1,1,5  v=1,1,3,1,1,1
//s=7,6,5,4,3,2,1,0  s=7,6,5,4,3,0  s=7,6,5,4,1,0  s=7,6,5,2,1,0  s=7,6,5,0  s=7,6,3,2,1,0
//7          8          9              10         11         12         13
//v=1,1,3,3  v=1,1,5,1  v=1,3,1,1,1,1  v=1,3,1,3  v=1,3,3,1  v=1,5,1,1  v=3,1,1,1,1,1
//s=7,6,3,0  s=7,6,1,0  s=7,4,3,2,1,0  s=7,4,3,0  s=7,4,1,0  s=7,2,1,0  s=5,4,3,2,1,0
//14         15         16         17     18         19     20
//v=3,1,1,3  v=3,1,3,1  v=3,3,1,1  v=3,5  v=5,1,1,1  v=5,3  v=8
//s=5,4,3,0  s=5,4,1,0  s=5,2,1,0  s=5,0  s=3,2,1,0  s=3,0  s=0

//total = 20
//c_arr = 0,1,2,0,10,0,6,0,1
}
)();


(function()
{
  console.time("Elapsed (full)");
  
  /*Распределение всевозможных комбинаций длин в фиксированном блоке*/
  var N = 8, /*длина блока*/
      m,      /*количество различных длин*/
      c = 0,  /*счетчик общего количества комбинаций*/
      s_arr,  /*массив распределений длин*/
      n_arr,  /*массив длин, по возрастанию, не превышающие N*/
      c_arr;  /*массив счетчиков количества комбинаций в зависимости от длины*/
  //n_arr = (new Uint16Array(8)).map((v,i)=>{return i+1;});
  n_arr = new Uint16Array([1,3,5,8]);/*зададим длины*/
  //n_arr = new Uint16Array([1,2,3,5,8,13,21,29]);/*зададим длины*/
  s_arr = new Uint8Array(N);
  c_arr = (new Array(N+1)).fill(0);
  n_arr.sort(); /*force sort by asc order*/
  m = n_arr.length;
    
  for (var s_len=1; s_len<=N; s_len++)
  {
    var j=0, S=N-s_len*n_arr[0]; s_arr.fill(0);
    while (j<s_len)
    {
      if(S==0)
      {/*когда распределение длин в сумме точно равны N*/
        c++;c_arr[s_len]++;
        if((c&0xFFFFFF)==0 || 0)console.log(c+": ("+(s_arr.map((v,i)=>{return i<s_len?n_arr[s_arr[i]]:0;}))+") = "+(N-S));
      }
	    j=0;S+=n_arr[s_arr[0]];
      if(S>n_arr[m-1])s_arr[j]=m;
      else {s_arr[0]++;if(s_arr[0]<m){S-=n_arr[s_arr[0]];continue;}}
      while (s_arr[j] == m) {
        s_arr[j]=0;S-=n_arr[0];
        j++;if(j>=s_len)break;if(S<0){S+=n_arr[s_arr[j]];s_arr[j]=m;}
        else{S+=n_arr[s_arr[j]];s_arr[j]++;if(s_arr[j]<m){S-=n_arr[s_arr[j]];break;}}
      }
    }
  }
  console.log("total = "+c);
  console.log("c_arr = "+c_arr);
  console.log("n_arr = "+n_arr);
  /*N=16 m=4 c=20569*/
  /*N=16 m=8 c=32192*/
  /*N=16 m=8 c=24219 n_arr=[1,2,3,4,6,8,10,13]*/
  /*N=16 m=8 c=12744 n_arr=[1,2,3,6,12,13,14,15]*/
  /*N=16 m=8 c=16298 n_arr=[1,2,3,5,7,10,13,14]*/
  /*N=32 m=8 c=2043730736 t=270 sec*/
  /*N=32 m=8 c= 431846836 n_arr=[1,2,3,5,8,13,21,29] t=69 sec*/
  /*N    m=8 c< 1<<(N-1)*/

  console.timeEnd("Elapsed (full)");  
}
)();

