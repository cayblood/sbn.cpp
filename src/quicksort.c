void swap(int *a, int *b)
{ 
  int t=*a; *a=*b; *b=t; 
}
void sort(int arr[], int beg, int end) 
{
  if (end > beg + 1) 
  {
    int piv = arr[beg], l = beg + 1, r = end;
    while (l != r-1) 
    {
       if(arr[l] <= piv) 
          l++;
       else 
          swap(&arr[l], &arr[r--]);
    }
    if(arr[l]<=piv && arr[r]<=piv)
       l=r+1;
    else if(arr[l]<=piv && arr[r]>piv)
       {l++; r--;}
    else if (arr[l]>piv && arr[r]<=piv)
       swap(&arr[l++], &arr[r--]);
    else
       r=l-1; 
   
    swap(&arr[r--], &arr[beg]);
    sort(arr, beg, r);
    sort(arr, l, end);
  }
}
