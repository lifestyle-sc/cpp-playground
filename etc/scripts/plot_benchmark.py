#!/usr/bin/env python3
import re
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.ticker import FuncFormatter
from scipy.optimize import curve_fit

# Full benchmark data with iterations
benchmark_data = """
BM_read_random<unsigned int>/1024               121 ns          120 ns      5673684 bytes_per_second=7.91431Gi/s items_per_second=2.12448G/s
BM_read_random<unsigned int>/2048               244 ns          243 ns      2816618 bytes_per_second=7.85982Gi/s items_per_second=2.10985G/s
BM_read_random<unsigned int>/4096               484 ns          483 ns      1441620 bytes_per_second=7.89972Gi/s items_per_second=2.12056G/s
BM_read_random<unsigned int>/8192              1120 ns         1087 ns       715153 bytes_per_second=7.0213Gi/s items_per_second=1.88477G/s
BM_read_random<unsigned int>/16384             2248 ns         2237 ns       352895 bytes_per_second=6.81976Gi/s items_per_second=1.83067G/s
BM_read_random<unsigned int>/32768             4372 ns         4362 ns       136793 bytes_per_second=6.9969Gi/s items_per_second=1.87822G/s
BM_read_random<unsigned int>/65536            13172 ns        12658 ns        56799 bytes_per_second=4.82173Gi/s items_per_second=1.29432G/s
BM_read_random<unsigned int>/131072           32883 ns        32792 ns        20137 bytes_per_second=3.72258Gi/s items_per_second=0.999273G/s
BM_read_random<unsigned int>/262144           84724 ns        84476 ns         8441 bytes_per_second=2.89006Gi/s items_per_second=775.795M/s
BM_read_random<unsigned int>/524288          196286 ns       195794 ns         3562 bytes_per_second=2.49385Gi/s items_per_second=669.438M/s
BM_read_random<unsigned int>/1048576         452845 ns       451424 ns         1542 bytes_per_second=2.16329Gi/s items_per_second=580.705M/s
BM_read_random<unsigned int>/2097152        1567036 ns      1557387 ns          460 bytes_per_second=1.2541Gi/s items_per_second=336.646M/s
BM_read_random<unsigned int>/4194304        9016861 ns      8906431 ns           85 bytes_per_second=449.114Mi/s items_per_second=117.732M/s
BM_read_random<unsigned int>/8388608       31748560 ns     31245594 ns           23 bytes_per_second=256.036Mi/s items_per_second=67.1183M/s
BM_read_random<unsigned int>/16777216      90173874 ns     88893071 ns            7 bytes_per_second=179.992Mi/s items_per_second=47.1837M/s
BM_read_random<unsigned int>/33554432     227615536 ns    225408291 ns            3 bytes_per_second=141.965Mi/s items_per_second=37.2152M/s
BM_read_random<unsigned int>/67108864     631778412 ns    625947311 ns            1 bytes_per_second=102.245Mi/s items_per_second=26.8029M/s
BM_read_random<unsigned int>/134217728   1453165771 ns   1439218722 ns            1 bytes_per_second=88.9371Mi/s items_per_second=23.3143M/s
BM_read_random<unsigned int>/268435456   3535518904 ns   3499266769 ns            1 bytes_per_second=73.1582Mi/s items_per_second=19.178M/s
BM_read_random<unsigned int>/536870912   7496294829 ns   7421316777 ns            1 bytes_per_second=68.9905Mi/s items_per_second=18.0854M/s
BM_read_random<unsigned int>/1073741824  1.8485e+10 ns   1.8270e+10 ns            1 bytes_per_second=56.0474Mi/s items_per_second=14.6925M/s
BM_read_random<unsigned long>/1024             62.8 ns         62.7 ns     11551180 bytes_per_second=15.206Gi/s items_per_second=2.04092G/s
BM_read_random<unsigned long>/2048              119 ns          118 ns      5836832 bytes_per_second=16.1256Gi/s items_per_second=2.16434G/s
BM_read_random<unsigned long>/4096              237 ns          237 ns      2959939 bytes_per_second=16.0995Gi/s items_per_second=2.16084G/s
BM_read_random<unsigned long>/8192              478 ns          477 ns      1476138 bytes_per_second=15.9864Gi/s items_per_second=2.14566G/s
BM_read_random<unsigned long>/16384            1037 ns         1035 ns       717945 bytes_per_second=14.7449Gi/s items_per_second=1.97902G/s
BM_read_random<unsigned long>/32768            2199 ns         2187 ns       321079 bytes_per_second=13.9524Gi/s items_per_second=1.87266G/s
BM_read_random<unsigned long>/65536            6306 ns         6292 ns       113600 bytes_per_second=9.70053Gi/s items_per_second=1.30198G/s
BM_read_random<unsigned long>/131072          17125 ns        17085 ns        40315 bytes_per_second=7.14503Gi/s items_per_second=958.989M/s
BM_read_random<unsigned long>/262144          43177 ns        43078 ns        16251 bytes_per_second=5.66747Gi/s items_per_second=760.675M/s
BM_read_random<unsigned long>/524288          99614 ns        99370 ns         7037 bytes_per_second=4.91376Gi/s items_per_second=659.513M/s
BM_read_random<unsigned long>/1048576        224574 ns       224017 ns         3086 bytes_per_second=4.35933Gi/s items_per_second=585.099M/s
BM_read_random<unsigned long>/2097152        566127 ns       562346 ns         1177 bytes_per_second=3.47317Gi/s items_per_second=466.161M/s
BM_read_random<unsigned long>/4194304       4627026 ns      4567707 ns          155 bytes_per_second=875.713Mi/s items_per_second=114.781M/s
BM_read_random<unsigned long>/8388608      15850377 ns     15583494 ns           49 bytes_per_second=513.364Mi/s items_per_second=67.2876M/s
BM_read_random<unsigned long>/16777216     44100656 ns     43557851 ns           16 bytes_per_second=367.328Mi/s items_per_second=48.1464M/s
BM_read_random<unsigned long>/33554432    123674624 ns    122544394 ns            5 bytes_per_second=261.13Mi/s items_per_second=34.2268M/s
BM_read_random<unsigned long>/67108864    321811195 ns    318973236 ns            2 bytes_per_second=200.644Mi/s items_per_second=26.2988M/s
BM_read_random<unsigned long>/134217728   823373995 ns    812497187 ns            1 bytes_per_second=157.539Mi/s items_per_second=20.649M/s
BM_read_random<unsigned long>/268435456  1870428083 ns   1854805015 ns            1 bytes_per_second=138.02Mi/s items_per_second=18.0905M/s
BM_read_random<unsigned long>/536870912  3988866491 ns   3950337473 ns            1 bytes_per_second=129.609Mi/s items_per_second=16.9881M/s
BM_read_random<unsigned long>/1073741824 9884608276 ns   9773952311 ns            1 bytes_per_second=104.768Mi/s items_per_second=13.7322M/s
BM_read_random<__m128i>/1024                   12.3 ns         12.2 ns     56522971 bytes_per_second=77.8536Gi/s items_per_second=5.22467G/s
BM_read_random<__m128i>/2048                   24.3 ns         24.2 ns     28495166 bytes_per_second=78.7982Gi/s items_per_second=5.28806G/s
BM_read_random<__m128i>/4096                   48.1 ns         48.0 ns     14527492 bytes_per_second=79.5393Gi/s items_per_second=5.33779G/s
BM_read_random<__m128i>/8192                   95.9 ns         95.7 ns      7264876 bytes_per_second=79.7376Gi/s items_per_second=5.3511G/s
BM_read_random<__m128i>/16384                   191 ns          191 ns      3654849 bytes_per_second=80.0042Gi/s items_per_second=5.36899G/s
BM_read_random<__m128i>/32768                   396 ns          394 ns      1790783 bytes_per_second=77.3838Gi/s items_per_second=5.19314G/s
BM_read_random<__m128i>/65536                   779 ns          776 ns       905198 bytes_per_second=78.609Gi/s items_per_second=5.27536G/s
BM_read_random<__m128i>/131072                 1566 ns         1562 ns       448236 bytes_per_second=78.1639Gi/s items_per_second=5.24549G/s
BM_read_random<__m128i>/262144                 3529 ns         3520 ns       199451 bytes_per_second=69.3511Gi/s items_per_second=4.65407G/s
BM_read_random<__m128i>/524288                 7031 ns         7016 ns        98457 bytes_per_second=69.5951Gi/s items_per_second=4.67045G/s
BM_read_random<__m128i>/1048576               14126 ns        14090 ns        49449 bytes_per_second=69.3091Gi/s items_per_second=4.65126G/s
BM_read_random<__m128i>/2097152               31319 ns        31124 ns        22558 bytes_per_second=62.7524Gi/s items_per_second=4.21124G/s
BM_read_random<__m128i>/4194304               62427 ns        62202 ns        11359 bytes_per_second=62.7995Gi/s items_per_second=4.2144G/s
BM_read_random<__m128i>/8388608              129244 ns       128688 ns         5451 bytes_per_second=60.7087Gi/s items_per_second=4.07409G/s
BM_read_random<__m128i>/16777216             353720 ns       351609 ns         1997 bytes_per_second=44.4385Gi/s items_per_second=2.98222G/s
BM_read_random<__m128i>/33554432             781646 ns       777304 ns          838 bytes_per_second=40.2031Gi/s items_per_second=2.69798G/s
BM_read_random<__m128i>/67108864            1602706 ns      1594934 ns          420 bytes_per_second=39.1866Gi/s items_per_second=2.62977G/s
BM_read_random<__m128i>/134217728           3208909 ns      3177549 ns          216 bytes_per_second=39.3385Gi/s items_per_second=2.63996G/s
BM_read_random<__m128i>/268435456           6472494 ns      6442931 ns           93 bytes_per_second=38.8022Gi/s items_per_second=2.60397G/s
BM_read_random<__m128i>/536870912          12906524 ns     12852053 ns           50 bytes_per_second=38.9043Gi/s items_per_second=2.61082G/s
BM_read_random<__m128i>/1073741824         25947771 ns     25850882 ns           26 bytes_per_second=38.6834Gi/s items_per_second=2.596G/s
BM_read_random<__m256i>/1024                   6.32 ns         6.30 ns    109242376 bytes_per_second=151.358Gi/s items_per_second=5.07874G/s
BM_read_random<__m256i>/2048                   12.4 ns         12.3 ns     56208374 bytes_per_second=154.558Gi/s items_per_second=5.18612G/s
BM_read_random<__m256i>/4096                   24.3 ns         24.2 ns     28671320 bytes_per_second=157.4Gi/s items_per_second=5.28148G/s
BM_read_random<__m256i>/8192                   48.0 ns         47.9 ns     14560653 bytes_per_second=159.225Gi/s items_per_second=5.34271G/s
BM_read_random<__m256i>/16384                  95.6 ns         95.5 ns      7277180 bytes_per_second=159.845Gi/s items_per_second=5.36351G/s
BM_read_random<__m256i>/32768                   193 ns          193 ns      3645656 bytes_per_second=158.168Gi/s items_per_second=5.30725G/s
BM_read_random<__m256i>/65536                   395 ns          394 ns      1764909 bytes_per_second=155.069Gi/s items_per_second=5.20327G/s
BM_read_random<__m256i>/131072                  777 ns          775 ns       908217 bytes_per_second=157.51Gi/s items_per_second=5.28516G/s
BM_read_random<__m256i>/262144                 1557 ns         1554 ns       447333 bytes_per_second=157.155Gi/s items_per_second=5.27324G/s
BM_read_random<__m256i>/524288                 3508 ns         3501 ns       199821 bytes_per_second=139.477Gi/s items_per_second=4.68009G/s
BM_read_random<__m256i>/1048576                7002 ns         6987 ns        97517 bytes_per_second=139.762Gi/s items_per_second=4.68964G/s
BM_read_random<__m256i>/2097152               14198 ns        14162 ns        48852 bytes_per_second=137.913Gi/s items_per_second=4.62758G/s
BM_read_random<__m256i>/4194304               30980 ns        30903 ns        22621 bytes_per_second=126.405Gi/s items_per_second=4.24144G/s
BM_read_random<__m256i>/8388608               61972 ns        61820 ns        11344 bytes_per_second=126.374Gi/s items_per_second=4.24042G/s
BM_read_random<__m256i>/16777216             129319 ns       128864 ns         5385 bytes_per_second=121.252Gi/s items_per_second=4.06855G/s
BM_read_random<__m256i>/33554432             358357 ns       356430 ns         1965 bytes_per_second=87.675Gi/s items_per_second=2.94188G/s
BM_read_random<__m256i>/67108864             796225 ns       789064 ns          836 bytes_per_second=79.2078Gi/s items_per_second=2.65777G/s
BM_read_random<__m256i>/134217728           1600081 ns      1592620 ns          419 bytes_per_second=78.487Gi/s items_per_second=2.63359G/s
BM_read_random<__m256i>/268435456           3193362 ns      3179614 ns          215 bytes_per_second=78.6259Gi/s items_per_second=2.63825G/s
BM_read_random<__m256i>/536870912           6605092 ns      6574767 ns           94 bytes_per_second=76.0483Gi/s items_per_second=2.55176G/s
BM_read_random<__m256i>/1073741824         12825097 ns     12766727 ns           50 bytes_per_second=78.3286Gi/s items_per_second=2.62827G/s
"""

# Parse the data
data_by_type = {}
for line in benchmark_data.strip().split('\n'):
    if not line.strip():
        continue

    # Extract type, size, time, and iterations
    match = re.match(r'BM_read_random<([^>]+)>/(\d+)\s+([\d.e+-]+)\s+ns\s+([\d.e+-]+)\s+ns\s+(\d+)', line)
    if match:
        dtype = match.group(1)
        size = int(match.group(2))
        time = float(match.group(3))
        iterations = int(match.group(5))

        if dtype not in data_by_type:
            data_by_type[dtype] = {'sizes': [], 'times': [], 'iterations': []}

        data_by_type[dtype]['sizes'].append(size)
        data_by_type[dtype]['times'].append(time)
        data_by_type[dtype]['iterations'].append(iterations)
