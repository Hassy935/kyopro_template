#pragma once

// ax+by=gcd(a,b)を満たす整数x,yを返す．
// 計算量はlogmin(|a|,|b|)．
// a=b=0の場合は{1,0}を返す．
// 入力がint範囲に収まらない場合には，オーバーフローに注意する．
pair<ll,ll> extgcd(ll a, ll b){
  ll s=a,t=b;

  ll xs=1,ys=0;
  ll xt=0,yt=1;

  while(t!=0){
    ll q=s/t;

    s%=t;
    xs-=q*xt;
    ys-=q*yt;

    swap(s,t);
    swap(xs,xt);
    swap(ys,yt);
  }

  if(s<0){
    xs=-xs;
    ys=-ys;
  }

  return {xs,ys};
}