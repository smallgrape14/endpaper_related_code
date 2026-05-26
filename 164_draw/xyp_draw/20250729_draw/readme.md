# 20250730

画图不被认可
- 使用自己推算的 多卡训练时间，然后计算各个占比，从而画图
	- 1.py 
		- **数据加载时间**真实数据变化(Ours和Baseline)
		- Ours相对于Baseline**数据加载时间**的降低百分比：(Baseline-Ours)/Baseline
	- 2.py
		- **数据加载时间**占据**整体时间**的百分比(Ours和Baseline)
	- 3.py
		- Ours相对于Baseline**端到端时间**减少的百分比：(Baseline-Ours)/Baseline

根据 250730_补充测试.xlsx 重新整理对应数据 [20250730_异步训练数据](https://docs.qq.com/sheet/DY2pjSEZOZXRpSnBu?no_promotion=1&tab=BB08J2)