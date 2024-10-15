import { useState } from 'react'
import './App.css'

function App() {
  const [order, setOrder] = useState('')

  const handleSubmit = (e) => {
    e.preventDefault()
    console.log('Order submitted:', order)
    // Here we'll add the logic to send the order and receive the encryption key
  }

  return (
    <div className="App">
      <h1>Disaster Relief Order System</h1>
      <form onSubmit={handleSubmit}>
        <textarea
          value={order}
          onChange={(e) => setOrder(e.target.value)}
          placeholder="Enter your order here"
          rows="4"
          cols="50"
        />
        <br />
        <button type="submit">Submit Order</button>
      </form>
    </div>
  )
}

export default App
