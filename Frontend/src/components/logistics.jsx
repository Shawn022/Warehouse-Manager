import React, { useMemo } from 'react'

const smallCard = (title, value, note) => (
  <div className="p-4 bg-white rounded-lg shadow-sm">
    <div className="text-sm text-gray-500">{title}</div>
    <div className="text-2xl font-bold">{value}</div>
    {note && <div className="text-xs text-gray-400">{note}</div>}
  </div>
)

const Logistics = () => {
  const [inventory, setInventory] = React.useState([]);
  const [loading, setLoading] = React.useState();
  const [outgoingOrders, setOutgoingOrders] = React.useState([]);
  const [restockOrders, setRestockOrders] = React.useState([]);
  const [error, setError] = React.useState(false);
  const [POWindow, setPOWindow] = React.useState(null);
  const maxLimit = 1000;
  const Priorities = [ "Low", "Medium", "High"]
  const [quantity, setQuantity] = React.useState(1);
  const [priority, setPriority] = React.useState(1);

  const loadData = async () => {
      try {
        setLoading(true);
        const res = await fetch('http://localhost:8080/inventory');
        const data = await res.json();
        setInventory(data);
      } catch (err) {
        console.error('Failed to fetch inventory data:', err);
        setError(true);
      } finally {
        setLoading(false);
      }
    }
    const loadOrders = async () => {
      try {
        const res = await fetch('http://localhost:8080/orders')
        if (!res.ok) throw new Error('Failed to load')
        const data = await res.json()
        setOutgoingOrders(data || [])
      } catch (err) {
        console.error('Failed to load orders:', err)
        setError(true);
      }
    }
    const loadReorders = async () => {
      try {
        const res = await fetch('http://localhost:8080/reorders')
        if (!res.ok) throw new Error('Failed to load')
        const data = await res.json()
        setRestockOrders(data || [])
      } catch (err) {
        console.error('Failed to load reorders:', err)
        setError(true);
      }
    }

  React.useEffect(() => {
    

    loadOrders();
    loadReorders();
    loadData();
  }, [])

  function findIncoming(sku){
    const order = restockOrders.filter(o => o.sku==sku).reduce((acc,o)=>acc+(o.qty||0),0);
    return order;
  }

  async function createRestock(reorder) {
    if (reorder.qty <= 0) return;
    reorder.date = (new Date()).toDateString()
    try {
      const res = await fetch('http://localhost:8080/reorders', {
        method: 'POST',
        mode: 'cors',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify(reorder)
      })
      if (!res.ok) throw new Error('Failed to create outgoing order')
      const data = await res.text()
      loadReorders();
    } catch (err) {
      console.error('Failed to create outgoing order:', err)
    }
  }

  function sendPO(qty,prior,item){
    const reorder={sku:item.sku,qty:String(qty),priority:String(prior),etaDays:prior*2,date:""};
    createRestock(reorder);
    setPOWindow(null);
    loadOrders();
    loadReorders();
  }

  function skuLimit(currSku) {
    if (!currSku) return maxLimit;
    let count = inventory.filter(x => x.sku.split('-')[0] == currSku.split('-')[0]).reduce((acc, i) => acc + i.quantity, 0);
    return (count > 0) ? maxLimit - count : 1;
  }

  // Derived KPIs
  const kpis = useMemo(() => {
    const totalSKUs = inventory.length
    const totalUnits = inventory.reduce((s, it) => s + (it.quantity || 0), 0)
    const lowStock = inventory.filter(it => (it.quantity ?? 0) <= (it.reorder ?? 0))
    const incomingStock = restockOrders.reduce((s, order) => s + (order.qty || 0), 0) || 0
    const incomingStockPrice = restockOrders.reduce((s, order) => {
      const item = inventory.find(it => it.sku === order.sku)
      return s + ((item?.price || 0) * (order.qty || 0))
    }, 0) || 0

    const outgoingStock = outgoingOrders.reduce((s, order) => s + (order.qty || 0), 0) || 0
    const outgoingStockPrice = outgoingOrders.reduce((s, order) => {
      const item = inventory.find(it => it.sku === order.sku)
      return s + ((item?.price || 0) * (order.qty || 0))
    }, 0) || 0
    const totalValue = inventory.reduce((s, it) => s + ((it.quantity || 0) * (it.price || 0)), 0)

    return { totalSKUs, totalUnits, lowStock, incomingStock, outgoingStock, totalValue, incomingStockPrice, outgoingStockPrice }
  }, [inventory])

  if (loading) {
    return <div className="loader"></div>
  }

  if (error) {
    return <div className="error">Failed to load data</div>
  }

  return (
    <>
      {POWindow!=null && <div className="po position-absolute h-120 w-3xl absolute left-90 top-30 rounded-3xl bg-white border shadow-lg p-6">
        <form>
          <label className="text-2xl font-bold ml-10 mt-10">Create Purchase Order</label>
          <div className="mt-10 ml-10">
            <h2 className="block text-gray-1000 text-xl">SKU: {POWindow.sku}</h2>
            <h2 className="block text-gray-1000 mt-4 text-lg">Name: {POWindow.name}</h2>

            <label className="block text-gray-700 mt-4" >Quantity:</label>
            <input type="number" min={1} max={skuLimit(POWindow.sku)} value={quantity} onChange={e => setQuantity(e.target.value)} className="border rounded w-64 py-2 px-3 mt-2" />
            
            <label className="block text-gray-700 mt-4">Priority:</label>
            <select className="border rounded w-64 py-2 px-3 mt-2" value={priority} onChange={e => setPriority(e.target.value)}>
              {Priorities.map((level, index) => (
                <option key={index} value={index+1}>{level}</option>
              ))}
            </select>
          </div>
          <div className="mt-10 ml-10">
            <button type="submit" className="bg-blue-500 text-white px-4 py-2 rounded mr-4" onClick={() => sendPO(quantity,priority,POWindow)}>Submit</button>
            <button type="button" className="bg-gray-300 text-black px-4 py-2 rounded" onClick={() =>setPOWindow(null) }>Cancel</button>
          </div>
        </form>
      </div>}
    <div>
      <h2 className="text-4xl font-bold">Logistics</h2>
      <p className="mt-4 text-gray-600">Overview of warehouse health and logistics KPIs.</p>

      <div className="grid grid-cols-1 md:grid-cols-3 gap-4 mt-6">
        {smallCard('Total SKUs', kpis.totalSKUs)}
        {smallCard('Total Units', kpis.totalUnits)}
        {smallCard('Low / Reorder', `${kpis.lowStock.length}`, 'Items at or below reorder point')}
        {smallCard('Incoming Stock', `${kpis.incomingStock}`, `($${kpis.incomingStockPrice.toFixed(2)})`)}
        {smallCard('Outgoing Stock', `${kpis.outgoingStock}`, `($${kpis.outgoingStockPrice.toFixed(2)})`)}
        {smallCard('Total Inventory Value', `$${kpis.totalValue.toFixed(2)}`)}
      </div>

      <div className="mt-8">
        <h3 className="text-2xl font-semibold">Low stock items</h3>
        {kpis.lowStock.length === 0 ? (
          <p className="mt-2 text-gray-500">All items are above reorder points.</p>
        ) : (
          <table className="w-full mt-4 table-auto border-collapse">
            <thead>
              <tr className="text-left border-b">
                <th className="py-2">SKU</th>
                <th className="py-2">Name</th>
                <th className="py-2">Quantity</th>
                <th className="py-2">Reorder Point</th>
                <th className="py-2">Incoming</th>
                <th className="py-2">Action</th>
              </tr>
            </thead>
            <tbody>
              {kpis.lowStock.map(it => (
                <tr key={it.sku} className="border-b">
                  <td className="py-2">{it.sku}</td>
                  <td className="py-2">{it.name}</td>
                  <td className="py-2">{it.quantity}</td>
                  <td className="py-2">{it.reorder}</td>
                  <td className="py-2">{findIncoming(it.sku)}</td>
                  <td className="py-2">
                    <button className="bg-blue-500 text-white px-2 py-1 rounded" onClick={() => setPOWindow(it)}>Create PO</button>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        )}
      </div>
    </div>
    </>
  )
}

export default Logistics

