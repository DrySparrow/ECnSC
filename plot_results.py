import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.ticker import FuncFormatter, LogLocator, NullFormatter

data = pd.read_csv('access_patterns.csv')

# Конвертация байтов в мегабайты для удобства
data['ArraySize(MB)'] = data['ArraySize(Bytes)'] / (1024 * 1024)

L1_SIZE = 48 * 1024           # 48 КБ в байтах
L2_SIZE = 512 * 1024          # 512 КБ в байтах
L3_SIZE = 6 * 1024 * 1024     # 6 МБ в байтах

plt.figure(figsize=(10, 8))

plt.plot(data['ArraySize(Bytes)'], data['Forward(ns)'], 'b-o',
         linewidth=1, markersize=2, label='forward', alpha=0.8)
plt.plot(data['ArraySize(Bytes)'], data['Backward(ns)'], 'g-s',
         linewidth=1, markersize=2, label='backward', alpha=0.8)
plt.plot(data['ArraySize(Bytes)'], data['Random(ns)'], 'r-^',
         linewidth=1, markersize=2, label='random', alpha=0.8)

plt.axvline(x=L1_SIZE, color='red', linestyle='--', alpha=0.7, linewidth=1.5, label='L1 кэш (48*4 КБ)')
plt.axvline(x=L2_SIZE, color='orange', linestyle='--', alpha=0.7, linewidth=1.5, label='L2 кэш (512*4 КБ)')
plt.axvline(x=L3_SIZE, color='purple', linestyle='--', alpha=0.7, linewidth=1.5, label='L3 кэш (6 МБ заявлено)')

plt.xlabel('Размер массива (байт)', fontsize=14)
plt.ylabel('Время доступа (тики)', fontsize=14)
plt.title('Зависимость времени доступа от размера массива', fontsize=16)
plt.grid(True, alpha=0.3, linestyle='--')
plt.legend(fontsize=12)

ax = plt.gca()
ax.set_xscale('log', base=2)
# Snap visible range to exact powers of two
min_x = data["ArraySize(Bytes)"].min()
max_x = data["ArraySize(Bytes)"].max()
min_pow = max(2, int(np.floor(np.log2(min_x))))  # start at least from 2^2
max_pow = int(np.ceil(np.log2(max_x)))
ax.set_xlim(2 ** min_pow, 2 ** max_pow)
# Major ticks at powers of two only
ax.xaxis.set_major_locator(LogLocator(base=2, subs=(1.0,), numticks=(max_pow - min_pow + 1)))
ax.xaxis.set_major_formatter(FuncFormatter(lambda x, _: f"2^{int(np.log2(x))}"))
# Hide minor tick labels to keep the axis clean
ax.xaxis.set_minor_formatter(NullFormatter())

plt.ylim(0, 50)
plt.yticks(np.arange(0, 51, 2))

plt.tight_layout()
plt.savefig('patterns_power_of_two.png', dpi=150, bbox_inches='tight')
plt.show()