(function()
{
  /*Распределение всевозможных комбинаций длин в фиксированном блоке*/
  var N = 16,    /*длина блока*/
      m = 8,     /*количество различных длин*/
      c = 0,     /*счетчик общего количества комбинаций*/
      s_arr = new Uint8Array(N), /*массив распределений длин*/
      n_arr = (new Uint16Array(m)).map((v,i)=>{return i+1;}),/*массив длин, по возрастанию, не превышающие N*/
      c_arr = (new Array(N+1)).fill(0); /*массив счетчиков количества комбинаций в зависимости от длины*/
  n_arr = new Uint16Array([1,2,3,4,6,8,10,13]);/*переопределим длины*/
  n_arr.sort(); /*force sort by asc order*/
  for (var s_len=1; s_len<=N; s_len++)
  {
    var j=0, S=N-s_len*n_arr[0]; s_arr.fill(0);
    while (j<s_len)
    {
      if(S==0)
      {/*когда распределение длин в сумме точно равны N*/
        c++;c_arr[s_len]++;
        if((c&0xFFFFFF)==0)console.log(c+": ("+(s_arr.map((v,i)=>{return i<s_len?n_arr[s_arr[i]]:0;}))+") = "+(N-S));
      }
	    j=0;S+=n_arr[s_arr[0]];s_arr[0]++;if(s_arr[0]<m){S-=n_arr[s_arr[0]];continue;}
      while (s_arr[j] == m) {
        s_arr[j]=0;S-=n_arr[0];
        j++;if(j>=s_len)break;if(S<0){S+=n_arr[s_arr[j]];s_arr[j]=m;}
        else{S+=n_arr[s_arr[j]];s_arr[j]++;if(s_arr[j]<m){S-=n_arr[s_arr[j]];break;}}
      }
    }
  }
  c_arr[0]=c;
  console.log("total = "+c);
  console.log("c_arr = "+c_arr);
  console.log("n_arr = "+n_arr);
  /*N=16 m=4 c=20569*/
  /*N=16 m=8 c=32192*/
  /*N=16 m=8 c=24219 n_arr=[1,2,3,4,6,8,10,13]*/
  /*N=16 m=8 c=12744 n_arr=[1,2,3,6,12,13,14,15]*/
  /*N=16 m=8 c=16298 n_arr=[1,2,3,5,7,10,13,14]*/
  /*N=32 m=8 c=2043730736 */
  /*N=32 m=8 c= 431846836 n_arr=[1,2,3,5,8,13,21,29]*/

}
)();
