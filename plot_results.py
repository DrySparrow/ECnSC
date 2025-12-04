import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.ticker import ScalarFormatter, FuncFormatter

data = pd.read_csv('access_ticks.csv')

plt.figure(figsize=(10, 8))

plt.plot(data['Size(el)'], data['Time(ticks)'], 'b-o', 
         linewidth=1, markersize=2, label='line', alpha=0.8)

plt.xlabel('Число фрагментов', fontsize=14)
plt.ylabel('Время доступа (тики)', fontsize=14)
plt.title('Зависимость времени доступа от размера массива', fontsize=16)
plt.grid(True, alpha=0.3, linestyle='--')
plt.legend(fontsize=12)


plt.xlim(0, 101)
plt.xticks(np.arange(5, 101, 5))

plt.ylim(0, 41)
plt.yticks(np.arange(0, 41, 5))

plt.tight_layout()
plt.savefig('patterns_power_of_two.png', dpi=150, bbox_inches='tight')
plt.show()